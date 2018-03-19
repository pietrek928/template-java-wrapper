#ifndef __DEFINE_H_
#define __DEFINE_H_

#ifndef __CONCAT
#define __CONCAT( x, y ) x ## y
#endif
#define CONCAT( x, y ) __CONCAT(x, y)

#define __STR(x) #x
#define STR(x) __STR(x)

#define ATTR_CHECKER(__Vname)                                          \
typedef struct {                                                        \
    template<class T>                                                    \
    static constexpr decltype(T::__Vname, true) test(T *p) {return true;} \
    static constexpr bool test(void *p) {return false;}                    \
} CONCAT( _tester_has_, __Vname );

#define hasattr(v, n) CONCAT( _tester_has_, n )::test(&(v))
#define hasattr_t(t, n) CONCAT( _tester_has_, n )::test((t*)NULL)

#define __FILE_INFO__ __FILE__ "(%s):" STR(__LINE__) ": "

// TODO: color ?
#define ERR( fmt, args...) fprintf(stderr, "error: " __FILE_INFO__ fmt "\n", __PRETTY_FUNCTION__, args);
#define WARN(fmt, args...) fprintf(stderr, "warning: "  __FILE_INFO__ fmt "\n", __PRETTY_FUNCTION__, args);
#define INFO(fmt, args...) fprintf(stderr, "info: " __FILE_INFO__ fmt "\n", __PRETTY_FUNCTION__, args);

#endif /* __DEFINE_H_ */

