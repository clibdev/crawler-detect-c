#ifndef CRAWLER_DETECT_H
#define CRAWLER_DETECT_H

typedef struct CrawlerDetect CrawlerDetect;

CrawlerDetect *crawler_detect_new(void);
void crawler_detect_free(CrawlerDetect *cd);
int crawler_detect_is_crawler(CrawlerDetect *cd, const char *ua);
const char *crawler_detect_get_match(const CrawlerDetect *cd);

#endif
