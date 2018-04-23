#include "package.h"

#include "teeest.h"

PACKAGE_ROOT(test,
    PACKAGE(teeest.test,
        CLASSN(teeest, teeest<int>,
            .constructor<>()
            .constructor<int>()
            .constructor<int,int>()
            .native<&teeest<int>::sumv<int,short>>("sumv")
            .native<&teeest<int>::sumv<int,short,int>>("sumv")
            .native<&ooo>("ooo")
            .native<&yyy>("yyy")
            .native<&kkk>("kkk")
            .native<&err_func>("err_func")
            .native<&err_func_strange>("err_func_strange")
            .native<get_ret_val>("get_ret_val")
        )
    )
)

