// Implemented by Vitaly "_Vi" Shukela in 2017; License = MIT or Apache 2.0

// Helper functions for C++

#ifndef _SCSP_CPP_HPP
#define _SCSP_CPP_HPP



#include <cstddef>
#include <cstdint>

#ifndef SCSP_INT
#define SCSP_INT int64_t
#endif

#ifndef SCSP_ENABLE_FLOAT
#define SCSP_ENABLE_FLOAT 1
#endif

#ifndef SCSP_EXPORT
#define SCSP_EXPORT
#endif

#ifndef SCSP_ENABLE_IOSTREAM
#define SCSP_ENABLE_IOSTREAM 1
#endif


#if SCSP_ENABLE_IOSTREAM
    #include <iosfwd>
#endif

namespace scsp {


class Callbacks;


#if SCSP_ENABLE_IOSTREAM
/** true means success */
bool SCSP_EXPORT parse_from_istream(
            std::istream& is,
            class Callbacks& callbacks);
#endif

/** true means success */
SCSP_INT SCSP_EXPORT parse_from_memory(
            const void *buffer,
            size_t count,
            class Callbacks& callbacks);

/** see description in scsp.h. */
SCSP_INT SCSP_EXPORT parse_from_fd(
            int fd, 
            class Callbacks& callbacks);

/** Parse bit-by-bit. see description in scsp.h */
SCSP_INT SCSP_EXPORT parse_lowlevel(
            class State& state,
            class Callbacks& callbacks,
            const void* buf,
            size_t count);




class  Callbacks {
    public:
    virtual void integer(SCSP_INT value) =0;
    
    virtual void bytestring_opened(SCSP_INT size_or_minus_one) =0;
    virtual void bytestring_chunk(const uint8_t* buf, size_t len) =0;
    virtual void bytestring_closed() =0;
    
    virtual void string_opened(SCSP_INT size_or_minus_one) =0;
    virtual void string_chunk(const uint8_t* buf, size_t len) =0;
    virtual void string_closed() =0;
    
    virtual void array_opened(SCSP_INT size_or_minus_one)=0;
    virtual void array_item()=0;
    virtual void array_closed()=0;
    
    virtual void map_opened(SCSP_INT size_or_minus_one)=0;
    virtual void map_key()=0;
    virtual void map_value()=0;
    virtual void map_closed()=0;
    
    // 'T' - true, 'F' - false, 'N' - null, 'U' - undefined
    virtual void simple (char value)=0;
    virtual void simple_other (SCSP_INT value)=0;
    virtual void tag (SCSP_INT value)=0;
#if SCSP_ENABLE_FLOAT
    virtual void noninteger (double value)=0;
#endif
}; 

class  CallbacksEmpty : public Callbacks {
    public:
    virtual void integer(SCSP_INT value) {}
    
    virtual void bytestring_opened(SCSP_INT size_or_minus_one) {}
    virtual void bytestring_chunk(const uint8_t* buf, size_t len) {}
    virtual void bytestring_closed() {}
    
    virtual void string_opened(SCSP_INT size_or_minus_one) {}
    virtual void string_chunk(const uint8_t* buf, size_t len) {}
    virtual void string_closed() {}
    
    virtual void array_opened(SCSP_INT size_or_minus_one){}
    virtual void array_item(){}
    virtual void array_closed(){}
    
    virtual void map_opened(SCSP_INT size_or_minus_one){}
    virtual void map_key(){}
    virtual void map_value(){}
    virtual void map_closed(){}
    
    virtual void simple (char value){}
    virtual void simple_other (SCSP_INT value){}
    virtual void tag (SCSP_INT value){}
#if SCSP_ENABLE_FLOAT
    virtual void noninteger (double value){}
#endif
}; 


class State;
class State* new_state();
void delete_state(class  State* state);


#if SCSP_ENABLE_IOSTREAM

/* Note: all strings, bytestrings, arrays and maps are streamed */
/* In contrary to the RFC, string chunks may split UTF-8 character's bytes apart */
class Generator : public Callbacks {
    public: 
    Generator(std::ostream &os) : o(os) {}
    
    private:
    std::ostream& o;
    
    void writething(uint8_t maj, uint64_t thing);
    
    virtual void integer(SCSP_INT value) ;
    
    virtual void bytestring_opened(SCSP_INT size_or_minus_one) ;
    virtual void bytestring_chunk(const uint8_t* buf, size_t len) ;
    virtual void bytestring_closed() ;
    
    virtual void string_opened(SCSP_INT size_or_minus_one) ;
    virtual void string_chunk(const uint8_t* buf, size_t len) ;
    virtual void string_closed() ;
    
    virtual void array_opened(SCSP_INT size_or_minus_one);
    virtual void array_item();
    virtual void array_closed();
    
    virtual void map_opened(SCSP_INT size_or_minus_one);
    virtual void map_key();
    virtual void map_value();
    virtual void map_closed();
    
    virtual void simple (char value);
    virtual void simple_other (SCSP_INT value);
    virtual void tag (SCSP_INT value);
#if SCSP_ENABLE_FLOAT
    virtual void noninteger (double value);
#endif
};

#endif

} // namespace scsp

#endif // _SCSP_CPP_HPP
