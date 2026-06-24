#include "crawler_detect.h"
#include "utest.h"

static void check_file(CrawlerDetect *cd, const char *path, const int expect, int *utest_result) {
    FILE *f = fopen(path, "rb");
    if (!f) {
        UTEST_PRINTF("  Cannot open: %s\n", path);
        *utest_result = UTEST_TEST_FAILURE;
        return;
    }

    fseek(f, 0, SEEK_END);
    const size_t file_size = (size_t) ftell(f);
    rewind(f);

    char *buf = malloc(file_size + 1);
    if (!buf) {
        fclose(f);
        *utest_result = UTEST_TEST_FAILURE;
        return;
    }

    const size_t nread = fread(buf, 1, file_size, f);
    fclose(f);
    buf[nread] = '\0';

    char *p = buf;
    while (*p) {
        char *line = p;
        while (*p && *p != '\n' && *p != '\r') {
            p++;
        }
        const size_t len = (size_t) (p - line);
        while (*p == '\r' || *p == '\n') {
            p++;
        }
        if (len == 0) {
            continue;
        }
        line[len] = '\0';

        const int rc = crawler_detect_is_crawler(cd, line);
        if (rc != expect) {
            UTEST_PRINTF("  %s: %s\n", expect ? "expected crawler" : "expected device", line);
            *utest_result = UTEST_TEST_FAILURE;
        }
    }

    free(buf);
}

struct UserAgentTest {
    CrawlerDetect *cd;
};

UTEST_F_SETUP(UserAgentTest) {
    utest_fixture->cd = crawler_detect_new();
    ASSERT_TRUE(utest_fixture->cd != NULL);
}

UTEST_F_TEARDOWN(UserAgentTest) {
    crawler_detect_free(utest_fixture->cd);
}

UTEST_F(UserAgentTest, user_agents_are_bots) {
    check_file(utest_fixture->cd, CRAWLER_DETECT_TESTS_DATA "/user_agent/crawlers.txt", 1, utest_result);
}

UTEST_F(UserAgentTest, user_agents_are_devices) {
    check_file(utest_fixture->cd, CRAWLER_DETECT_TESTS_DATA "/user_agent/devices.txt", 0, utest_result);
}

UTEST_F(UserAgentTest, sec_ch_ua_are_bots) {
    check_file(utest_fixture->cd, CRAWLER_DETECT_TESTS_DATA "/sec_ch_ua/crawlers.txt", 1, utest_result);
}

UTEST_F(UserAgentTest, sec_ch_ua_are_devices) {
    check_file(utest_fixture->cd, CRAWLER_DETECT_TESTS_DATA "/sec_ch_ua/devices.txt", 0, utest_result);
}

UTEST_F(UserAgentTest, returns_correct_matched_bot_name) {
    crawler_detect_is_crawler(utest_fixture->cd,
                              "Mozilla/5.0 (iPhone; CPU iPhone OS 7_1 like Mac OS X) "
                              "AppleWebKit (KHTML, like Gecko) Mobile (compatible; Yahoo Ad monitoring; "
                              "https://help.yahoo.com/kb/yahoo-ad-monitoring-SLN24857.html)");

    const char *match = crawler_detect_get_match(utest_fixture->cd);
    ASSERT_TRUE(match != NULL);
    ASSERT_STREQ("monitoring", match);
}

UTEST_F(UserAgentTest, returns_full_matched_bot_name) {
    crawler_detect_is_crawler(utest_fixture->cd, "somenaughtybot");
    ASSERT_STREQ("somenaughtybot", crawler_detect_get_match(utest_fixture->cd));
}

UTEST_F(UserAgentTest, returns_null_when_no_bot_detected) {
    crawler_detect_is_crawler(utest_fixture->cd, "nothing to see here");
    ASSERT_TRUE(crawler_detect_get_match(utest_fixture->cd) == NULL);
}

UTEST_F(UserAgentTest, empty_user_agent) {
    ASSERT_EQ(0, crawler_detect_is_crawler(utest_fixture->cd, "      "));
}

UTEST_F(UserAgentTest, matches_do_not_persist_across_calls) {
    CrawlerDetect *cd = utest_fixture->cd;
    const char *bot =
        "Mozilla/5.0 (iPhone; CPU iPhone OS 7_1 like Mac OS X) "
        "AppleWebKit (KHTML, like Gecko) Mobile (compatible; Yahoo Ad monitoring; "
        "https://help.yahoo.com/kb/yahoo-ad-monitoring-SLN24857.html)";

    crawler_detect_is_crawler(cd, bot);
    ASSERT_STREQ("monitoring", crawler_detect_get_match(cd));

    crawler_detect_is_crawler(cd, "This should not match");
    ASSERT_TRUE(crawler_detect_get_match(cd) == NULL);

    /* empty UA after a positive match clears the stored match */
    crawler_detect_is_crawler(cd, bot);
    crawler_detect_is_crawler(cd, "");
    ASSERT_TRUE(crawler_detect_get_match(cd) == NULL);

    /* excluded-only UA after a positive match clears the stored match */
    crawler_detect_is_crawler(cd, bot);
    crawler_detect_is_crawler(cd, "iPod");
    ASSERT_TRUE(crawler_detect_get_match(cd) == NULL);
}

UTEST_F(UserAgentTest, amazon_cloudfront_not_a_crawler) {
    ASSERT_EQ(0, crawler_detect_is_crawler(utest_fixture->cd, "Amazon CloudFront"));
}
