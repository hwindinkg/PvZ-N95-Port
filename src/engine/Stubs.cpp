// engine/Stubs.cpp - Stubs for port-specific and missing runtime functions
#include "SexyAppBase.h"
#include "Image.h"
#include "../Lawn/LawnCommon.h"

namespace Sexy { SexyAppBase* gSexyAppBase = NULL; }

// Math stubs (not in Symbian libc)
extern "C" double fmod(double x, double y) { (void)x; (void)y; return 0.0; }
extern "C" int rand() { return 0; }
extern "C" int abs(int x) { return x < 0 ? -x : x; }
extern "C" int strncmp(const char* s1, const char* s2, unsigned int n) {
    while (n--) { if (*s1 != *s2) return (unsigned char)*s1 - (unsigned char)*s2; if (!*s1) return 0; s1++; s2++; }
    return 0;
}

void DrawSeedPacket(Sexy::Graphics* g, float x, float y, int st, int it, float al, int gr, bool sel, bool im) {
    (void)g; (void)x; (void)y; (void)st; (void)it; (void)al; (void)gr; (void)sel; (void)im;
}

// RTTI vtable stubs (libsupc++ missing them)
namespace __cxxabiv1 {
    class __class_type_info { virtual void _d1() {} };
    class __si_class_type_info : public __class_type_info { virtual void _d2() {} };
    class __vmi_class_type_info : public __class_type_info { virtual void _d3() {} };
    __class_type_info __ci; __si_class_type_info __si; __vmi_class_type_info __vi;
}

// libsupc++ function stubs
extern "C" void* __dynamic_cast(const void* s, const void* st, const void* dt, int o) { (void)s; (void)st; (void)dt; (void)o; return NULL; }
extern "C" void __cxa_pure_virtual() { User::Panic(_L("pv"), 0); }

// Symbian exception globals
struct TCppRTExceptionsGlobals { TCppRTExceptionsGlobals(); } TCppRTExceptionsGlobalsInstance;
TCppRTExceptionsGlobals::TCppRTExceptionsGlobals() {}

// XLeaveException typeinfo symbols
extern "C" const void* _ZTI15XLeaveException = 0;
extern "C" const char _ZTS15XLeaveException[] = "15XLeaveException";

// CallThrdProcEntry - process entry point (TInt return, creates CTrapCleanup)
extern "C" TInt E32Main();
extern "C" __attribute__((visibility("hidden"))) TInt CallThrdProcEntry() {
    CTrapCleanup* cleanup = CTrapCleanup::New();
    User::InitProcess();
    TInt error = E32Main();
    delete cleanup;
    return error;
}
extern "C" __attribute__((visibility("hidden"))) void __cpp_initialize__aeabi_() { }
extern "C" void* _Znwj(unsigned int);extern "C" void* _Znaj(unsigned int);extern "C" void _ZdlPv(void*);extern "C" void _ZdaPv(void*);static void* __force_new[] = { (void*)_Znwj, (void*)_Znaj, (void*)_ZdlPv, (void*)_ZdaPv };

// Software float IEEE 754 implementations
// Uses integer bit manipulation to avoid circular aeabi calls
typedef unsigned int u32;
typedef unsigned long long u64;

static u32 float_pack(u32 s, int e, u32 m) {
    if (m == 0 && e < -125) e = -126;
    if (e > 127) { e = 128; m = 0; }
    if (e < -126) { e = -126; m = 0; }
    return (s << 31) | ((e + 127) << 23) | (m & 0x7FFFFF);
}
static void float_unpack(u32 f, u32* s, int* e, u32* m) {
    *s = f >> 31; *e = ((f >> 23) & 0xFF) - 127; *m = f & 0x7FFFFF;
    if (*e != -127) *m |= 0x800000; else *e = -126;
}

extern "C" float __aeabi_fadd(float a, float b) {
    u32 sa,sb;int ea,eb;u32 ma,mb;
    float_unpack(*(u32*)&a,&sa,&ea,&ma);
    float_unpack(*(u32*)&b,&sb,&eb,&mb);
    if(ea<eb){u32 t;int te;te=ea;ea=eb;eb=te;t=sa;sa=sb;sb=t;t=ma;ma=mb;mb=t;}
    int d=ea-eb;if(d>32)d=32;
    u32 m;int e=ea;
    if(sa==sb){m=ma+(mb>>d);if(m&0x1000000){m>>=1;e++;}}
    else{m=ma-(mb>>d);while(m&&!(m&0x800000)){m<<=1;e--;}}
    u32 r=float_pack(sa,e,m);
    return *(float*)&r;
}
extern "C" float __aeabi_fsub(float a, float b) {
    u32 bi=*(u32*)&b;bi^=0x80000000;
    return __aeabi_fadd(a,*(float*)&bi);
}
extern "C" float __aeabi_fmul(float a, float b) {
    u32 sa,sb;int ea,eb;u32 ma,mb;
    float_unpack(*(u32*)&a,&sa,&ea,&ma);
    float_unpack(*(u32*)&b,&sb,&eb,&mb);
    u64 m=(u64)ma*mb;int e=ea+eb-127;
    if(m&0x100000000ULL){m>>=1;e++;}
    u32 r=float_pack(sa^sb,e,(u32)(m>>24));
    return *(float*)&r;
}
extern "C" float __aeabi_fdiv(float a, float b) {
    u32 sa,sb;int ea,eb;u32 ma,mb;
    float_unpack(*(u32*)&a,&sa,&ea,&ma);
    float_unpack(*(u32*)&b,&sb,&eb,&mb);
    u64 m=((u64)ma<<31)/mb;
    u32 r=float_pack(sa^sb,ea-eb+127,(u32)(m>>8));
    return *(float*)&r;
}
extern "C" int __aeabi_fcmpeq(float a, float b) { return *(u32*)&a==*(u32*)&b; }
extern "C" int __aeabi_fcmplt(float a, float b) { return *(u32*)&a<*(u32*)&b; }
extern "C" int __aeabi_fcmple(float a, float b) { return *(u32*)&a<=*(u32*)&b; }
extern "C" int __aeabi_fcmpge(float a, float b) { return *(u32*)&a>=*(u32*)&b; }
extern "C" int __aeabi_fcmpgt(float a, float b) { return *(u32*)&a>*(u32*)&b; }
extern "C" float __aeabi_i2f(int x) {
    if(!x)return 0;u32 s=x<0?1:0;u32 m=s?-x:x;int e=0;
    while(m>0x7FFFFF){m>>=1;e++;}while(m<0x800000&&e>-126){m<<=1;e--;}
    u32 r=float_pack(s,e,m);return *(float*)&r;
}
extern "C" float __aeabi_ui2f(u32 x) {
    int e=0;while(x>0x7FFFFF){x>>=1;e++;}while(x<0x800000&&e>-126){x<<=1;e--;}
    u32 r=float_pack(0,e,x);return *(float*)&r;
}
extern "C" int __aeabi_f2iz(float a) {
    u32 f=*(u32*)&a;int e=((f>>23)&0xFF)-127;if(e<0)return 0;
    if(e>30)return(f>>31)?0x80000000:0x7FFFFFFF;
    u32 m=(f&0x7FFFFF)|0x800000;if(e<=23)m>>=(23-e);else m<<=(e-23);
    if(f>>31)m=~m+1;return(int)m;
}
extern "C" double __aeabi_f2d(float a) {
    u32 f=*(u32*)&a;u32 s=f>>31;int e=((f>>23)&0xFF)-127;
    u32 m=(f&0x7FFFFF)|0x800000;
    u64 r=((u64)s<<63)|((u64)(e+1023)<<52)|((u64)m<<29);
    return *(double*)&r;
}
extern "C" float __aeabi_d2f(double d) {
    u64 dd=*(u64*)&d;u32 s=dd>>63;int e=((dd>>52)&0x7FF)-1023;
    u64 m=(dd&0xFFFFFFFFFFFFFULL)|(1ULL<<52);
    int g=29-e;if(g>=0)m>>=g;else m<<=(-g);
    u32 r=float_pack(s,e,(u32)m);
    return *(float*)&r;
}
extern "C" int __aeabi_d2iz(double d){float f=__aeabi_d2f(d);return __aeabi_f2iz(f);}
extern "C" double __aeabi_i2d(int x){return __aeabi_f2d(__aeabi_i2f(x));}
extern "C" double __aeabi_dadd(double a,double b){return __aeabi_f2d(__aeabi_fadd(__aeabi_d2f(a),__aeabi_d2f(b)));}
extern "C" double __aeabi_dsub(double a,double b){return __aeabi_f2d(__aeabi_fsub(__aeabi_d2f(a),__aeabi_d2f(b)));}
extern "C" double __aeabi_dmul(double a,double b){return __aeabi_f2d(__aeabi_fmul(__aeabi_d2f(a),__aeabi_d2f(b)));}
extern "C" double __aeabi_ddiv(double a,double b){return __aeabi_f2d(__aeabi_fdiv(__aeabi_d2f(a),__aeabi_d2f(b)));}
extern "C" int __aeabi_dcmpeq(double a,double b){return *(u64*)&a==*(u64*)&b;}
extern "C" int __aeabi_dcmplt(double a,double b){u64 ad=*(u64*)&a,bd=*(u64*)&b;if((ad^bd)>>63)return bd>>63;return ad<bd;}
extern "C" int __aeabi_dcmple(double a,double b){u64 ad=*(u64*)&a,bd=*(u64*)&b;if((ad^bd)>>63)return bd>>63;return ad<=bd;}
extern "C" int __aeabi_dcmpgt(double a,double b){u64 ad=*(u64*)&a,bd=*(u64*)&b;if((ad^bd)>>63)return ad>>63;return ad>bd;}
extern "C" int __aeabi_dcmpge(double a,double b){u64 ad=*(u64*)&a,bd=*(u64*)&b;if((ad^bd)>>63)return ad>>63;return ad>=bd;}
extern "C" void abort() { User::Panic(_L("abort"), 0); }
