#define PCRE2_CODE_UNIT_WIDTH 8
#include "crawler_detect.h"
#include <ctype.h>
#include <pcre2.h>
#include <stdlib.h>
#include <string.h>
#include "fixtures/fixtures.h"

#define MATCH_BUF_SIZE 512

struct CrawlerDetect {
    pcre2_code *crawlers_re;
    pcre2_code *exclusions_re;
    pcre2_match_data *match_data;
    pcre2_match_data *excl_match_data;
    char *buf;
    size_t buf_cap;
    char match[MATCH_BUF_SIZE];
};

static pcre2_code *compile_combined(const char *const patterns[], const size_t count) {
    // (?:p1|p2|...|pN)\0
    size_t total = 5 + (count - 1);  // "(?:" + (count-1) '|' separators + ")" + '\0'
    for (size_t i = 0; i < count; ++i) {
        total += strlen(patterns[i]);
    }

    unsigned char *combined = malloc(total);
    if (!combined) {
        return NULL;
    }

    unsigned char *p = combined;
    *p++ = '(';
    *p++ = '?';
    *p++ = ':';
    for (size_t i = 0; i < count; ++i) {
        if (i > 0) {
            *p++ = '|';
        }
        const size_t len = strlen(patterns[i]);
        memcpy(p, patterns[i], len);
        p += len;
    }
    *p++ = ')';
    *p = '\0';

    int errcode = 0;
    size_t erroffset = 0;
    pcre2_code *re = pcre2_compile(combined, PCRE2_ZERO_TERMINATED, PCRE2_CASELESS, &errcode, &erroffset, NULL);

    free(combined);

    if (re) {
        pcre2_jit_compile(re, PCRE2_JIT_COMPLETE);
    }

    return re;
}

CrawlerDetect *crawler_detect_new(void) {
    CrawlerDetect *cd = calloc(1, sizeof(*cd));
    if (!cd) {
        return NULL;
    }

    cd->crawlers_re = compile_combined(CRAWLER_DETECT_CRAWLERS, CRAWLER_DETECT_CRAWLERS_COUNT);
    cd->exclusions_re = compile_combined(CRAWLER_DETECT_EXCLUSIONS, CRAWLER_DETECT_EXCLUSIONS_COUNT);

    cd->match_data = pcre2_match_data_create_from_pattern(cd->crawlers_re, NULL);
    cd->excl_match_data = pcre2_match_data_create_from_pattern(cd->exclusions_re, NULL);

    if (!cd->crawlers_re || !cd->exclusions_re || !cd->match_data || !cd->excl_match_data) {
        crawler_detect_free(cd);
        return NULL;
    }

    return cd;
}

void crawler_detect_free(CrawlerDetect *cd) {
    if (cd) {
        free(cd->buf);
        pcre2_match_data_free(cd->excl_match_data);
        pcre2_match_data_free(cd->match_data);
        pcre2_code_free(cd->crawlers_re);
        pcre2_code_free(cd->exclusions_re);
        free(cd);
    }
}

int crawler_detect_is_crawler(CrawlerDetect *cd, const char *ua) {
    cd->match[0] = '\0';

    if (!ua || !*ua) {
        return 0;
    }

    const size_t ua_len = strlen(ua);
    const size_t needed = ua_len + 1;
    if (needed > cd->buf_cap) {
        char *tmp = realloc(cd->buf, needed);
        if (!tmp) {
            return 0;
        }
        cd->buf = tmp;
        cd->buf_cap = needed;
    }

    size_t out_len = cd->buf_cap;
    const int sub_rc = pcre2_substitute(cd->exclusions_re, (PCRE2_SPTR) ua, ua_len, 0, PCRE2_SUBSTITUTE_GLOBAL,
                                        cd->excl_match_data, NULL, (PCRE2_SPTR) "", 0, (PCRE2_UCHAR *) cd->buf,
                                        &out_len);

    if (sub_rc < 0) {
        memcpy(cd->buf, ua, ua_len + 1);
    }

    /* trim leading whitespace */
    char *start = cd->buf;
    while (*start && isspace((unsigned char) *start)) {
        start++;
    }

    if (*start == '\0') {
        return 0;
    }

    /* trim trailing whitespace */
    char *end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char) *end)) {
        *end-- = '\0';
    }

    const int rc = pcre2_match(cd->crawlers_re, (PCRE2_SPTR) start, PCRE2_ZERO_TERMINATED, 0, 0, cd->match_data, NULL);

    if (rc > 0) {
        const PCRE2_SIZE *const ov = pcre2_get_ovector_pointer(cd->match_data);
        size_t mlen = ov[1] - ov[0];
        if (mlen >= MATCH_BUF_SIZE) {
            mlen = MATCH_BUF_SIZE - 1;
        }
        memcpy(cd->match, start + ov[0], mlen);
        cd->match[mlen] = '\0';
    }

    return rc > 0 ? 1 : 0;
}

const char *crawler_detect_get_match(const CrawlerDetect *cd) {
    return cd->match[0] ? cd->match : NULL;
}
