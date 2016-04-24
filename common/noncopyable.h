#ifndef COMMON_NONCOPYABLE_H_
#define COMMON_NONCOPYABLE_H_

#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
    TypeName(const TypeName&);             \
    void operator=(const TypeName&)		   

#endif