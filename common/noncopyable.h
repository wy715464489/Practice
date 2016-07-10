// Copyright [2012-2014] <HRG>
#ifndef COMMON_NONCOPYABLE_H_
#define COMMON_NONCOPYABLE_H_

 namespace common {

// A macro to disallow the copy constructor and operator= functions
// This should be used in the private: declarations for a class
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
    TypeName(const TypeName&);             \
    void operator=(const TypeName&)

}  // namespace common

#endif  // COMMON_NONCOPYABLE_H_
