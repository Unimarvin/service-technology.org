#pragma once

#if __cplusplus >= 201103L
#define __cplusplus11
#endif

#ifndef nullptr
#define nullptr NULL
#endif

#include <string>
#include <vector>
#include <map>
#ifdef __cplusplus11
#include <mutex>
#include <initializer_list>
#endif

class JSON {
    private:
#ifdef __cplusplus11
        /// mutex to guard payload
        static std::mutex _token;
#endif

    public:
        /// possible types of a JSON object
        typedef enum {
            array, object, null, string, boolean, number_int, number_float
        } json_t;

    private:
        /// the type of this object
        json_t _type;

        /// the payload
        void *_payload;

    public:
#ifdef __cplusplus11
        /// a type for objects
        typedef std::tuple<std::string, JSON> object_t;
        /// a type for arrays
        typedef std::initializer_list<JSON> array_t;
#endif

    public:
        /// create an empty (null) object
        JSON();
        /// create a string object from C++ string
        JSON(const std::string&);
        /// create a string object from C string
        JSON(char*);
        /// create a string object from C string
        JSON(const char*);
        /// create a Boolean object
        JSON(const bool);
        /// create a number object
        JSON(const int);
        /// create a number object
        JSON(const double);
#ifdef __cplusplus11
        /// create from an initializer list (to an array)
        JSON(array_t);
        /// create from a mapping (to an object)
        JSON(object_t);
#endif

        /// copy constructor
        JSON(const JSON&);

#ifdef __cplusplus11
        /// move constructor
        JSON(JSON &&);
#endif

        /// copy assignment
#ifdef __cplusplus11
        JSON &operator=(JSON);
#else
        JSON &operator=(const JSON &);
#endif

        /// destructor
        ~JSON();

        /// implicit conversion to string representation
        operator const std::string() const;
        /// implicit conversion to integer (only for numbers)
        operator int() const;
        /// implicit conversion to double (only for numbers)
        operator double() const;
        /// implicit conversion to Boolean (only for Booleans)
        operator bool() const;

        /// write to stream
        friend std::ostream& operator<<(std::ostream &o, const JSON &j) {
            o << j.toString();
            return o;
        }

        /// read from stream
        friend std::istream& operator>>(std::istream &i, JSON &j) {
            j.parseStream(i);
            return i;
        }

        /// explicit conversion to string representation (C++ style)
        const std::string toString() const;

        /// add an object/array to an array
        JSON& operator+=(const JSON&);
        /// add a string to an array
        JSON& operator+=(const std::string&);
        /// add a string to an array
        JSON& operator+=(const char *);
        /// add a Boolean to an array
        JSON& operator+=(bool);
        /// add a number to an array
        JSON& operator+=(int);
        /// add a number to an array
        JSON& operator+=(double);

        /// operator to set an element in an array
        JSON& operator[](int);
        /// operator to get an element in an array
        const JSON& operator[](const int) const;

        /// operator to set an element in an object
        JSON& operator[](const std::string&);
        /// operator to set an element in an object
        JSON& operator[](const char*);
        /// operator to get an element in an object
        const JSON& operator[](const std::string&) const;

        /// return the number of stored values
        size_t size() const;
        /// checks whether object is empty
        bool empty() const;

        /// return the type of the object
        json_t type() const;

        /// lexicographically compares the values
        bool operator==(const JSON&) const;

    private:
        /// return the type as string
        std::string _typename() const;

        // additional parser functions
        void parseStream(std::istream&);
        void parseError(unsigned int pos, std::string tok) const;
        bool checkDelim(char*& buf, unsigned int& len, char tok) const;
        std::string getString(char*& buf, unsigned int& len, unsigned int max) const;
        void parse(char*& buf, unsigned int& len, unsigned int max);

        /*
    public:
        class iterator {
            public:
                iterator();
                iterator(JSON *);
                iterator(const iterator&);
                ~iterator();

                iterator& operator=(const iterator&);
                bool operator==(const iterator&) const;
                bool operator!=(const iterator&) const;
                iterator& operator++();
                JSON& operator*() const;
                JSON* operator->() const;

            private:
                JSON *_object = nullptr;
                std::vector<JSON>::iterator *_vi = nullptr;
                std::map<std::string, JSON>::iterator *_oi = nullptr;
        };

    public:
        iterator begin();
        iterator end();
        */
};
