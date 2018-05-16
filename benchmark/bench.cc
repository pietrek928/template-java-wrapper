// indicates that file implements a java package
#include <package.h>

// wrapped class
#include "bench.h"

// package definition
PACKAGE_ROOT(test,
    CLASS(bench, // class wrapping
        .constructor<>()
        .native<&test1>("test1")
        .native<&test2>("test2")
        .native<&test3>("test3")
        .native<&test4>("test4")
        .native<&test5>("test5")
    )
)

