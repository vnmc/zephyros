#ifdef __cplusplus
extern "C" {
#endif
#define BIN_DATA(_NAME) \
    extern char _NAME##_begin; \
    extern int _NAME##_len
#ifdef __cplusplus
}
#endif

BIN_DATA(__res0);
BIN_DATA(__res1);
BIN_DATA(__res2);
BIN_DATA(__res3);
BIN_DATA(__res4);

void SetResources()
{
    Zephyros::SetResource("app/main.js~", &__res0_begin, __res0_len);
    Zephyros::SetResource("app/style.css", &__res1_begin, __res1_len);
    Zephyros::SetResource("app/index.html", &__res2_begin, __res2_len);
    Zephyros::SetResource("app/main.js", &__res3_begin, __res3_len);
    Zephyros::SetResource("app/zepto.js", &__res4_begin, __res4_len);
}

