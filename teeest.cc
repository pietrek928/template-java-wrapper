#include "package.h"

#include "teeest.h"

PACKAGE_ROOT(test,
    PACKAGE(teeest.test,
        CLASSN(teeest, teeest<int>,
            .constructor<>()
            .constructor<int>()
            .constructor<int,int>()
            .native<&teeest<int>::sumv<int,unsigned char>>("sumv")
            .native<&teeest<int>::sumv<int,unsigned char,int>>("sumv")
            .native<&ooo>("ooo")
            .native<&yyy>("yyy")
            .native<&kkk>("kkk")
        )
    )
)

