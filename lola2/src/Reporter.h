/*!
\file Reporter.h
\status approved 25.01.2012
*/

#pragma once

#include "Socket.h"

/// error codes for the Reporter::abort() function
typedef enum
{
    ERROR_SYNTAX,       ///< syntax error
    ERROR_COMMANDLINE,  ///< wrong commandline parameter
    ERROR_FILE          ///< file input/output error
} errorcode_t;

/// markup types for Reporter::markup() function
typedef enum
{
    MARKUP_TOOL,      ///< markup the name of a tool
    MARKUP_FILE,      ///< markup the name of a file
    MARKUP_OUTPUT,    ///< markup the type of a file
    MARKUP_GOOD,      ///< markup some good output
    MARKUP_BAD,       ///< markup some bad output
    MARKUP_WARNING,   ///< markup a warning
    MARKUP_IMPORTANT, ///< markup an important message
    MARKUP_PARAMETER  ///< markup a command-line parameter
} markup_t;


/*!
\brief Class to implement reporting functionality
*/
class Reporter
{
    protected:
        /// string class to avoid STL's std::string
        class String
        {
            private:
                /// payload - is freed in destructor
                char* s;

            public:
                /// constructor (does only copy pointer, not content)
                String(char* s);

                /// destructor - frees payload
                ~String();

                /// getter for s
                char* str() const;
        };

        /// error messages
        static const char* error_messages[];

    public:
        virtual ~Reporter() {}

        /// markup a string
        virtual Reporter::String markup(markup_t, const char*, ...) const = 0;

        /// always report
        virtual void message(const char*, ...) const = 0;

        /// only report in verbose mode
        virtual void status(const char*, ...) const = 0;

        /// display error message and abort program
        __attribute__((noreturn)) virtual void abort(errorcode_t) const = 0;
};

class ReporterSocket : public Reporter
{
    private:
        /// whether verbose reports are desired
        bool verbose;

        /// socket for this reporter
        Socket mySocket;

    public:
        ReporterSocket(u_short port, const char* ip, bool = true);
        ~ReporterSocket();

        void message(const char*, ...) const;
        void status(const char*, ...) const;
        __attribute__((noreturn)) void abort(errorcode_t) const;
        Reporter::String markup(markup_t, const char*, ...) const;
};

class ReporterStream : public Reporter
{
    private:
        /// whether verbose reports are desired
        bool verbose;

        /// whether to use colored output
        const bool useColor;

        /// set foreground color to red
        const char* _cr_;
        /// set foreground color to green
        const char* _cg_;
        /// set foreground color to yellow
        const char* _cy_;
        /// set foreground color to blue
        const char* _cb_;
        /// set foreground color to magenta
        const char* _cm_;
        /// set foreground color to cyan
        const char* _cc_;
        /// set foreground color to light grey
        const char* _cl_;

        /// set foreground color to red (underlined)
        const char* _cr__;
        /// set foreground color to green (underlined)
        const char* _cg__;
        /// set foreground color to yellow (underlined)
        const char* _cy__;
        /// set foreground color to blue (underlined)
        const char* _cb__;
        /// set foreground color to magenta (underlined)
        const char* _cm__;
        /// set foreground color to cyan (underlined)
        const char* _cc__;
        /// set foreground color to light grey (underlined)
        const char* _cl__;

        /// set foreground color to red (bold)
        const char* _cR_;
        /// set foreground color to green (bold)
        const char* _cG_;
        /// set foreground color to yellow (bold)
        const char* _cY_;
        /// set foreground color to blue (bold)
        const char* _cB_;
        /// set foreground color to magenta (bold)
        const char* _cM_;
        /// set foreground color to cyan (bold)
        const char* _cC_;
        /// set foreground color to light grey (bold)
        const char* _cL_;

        /// reset foreground color
        const char* _c_;
        /// make text bold
        const char* _bold_;
        /// unterline text
        const char* _underline_;

    public:
        ReporterStream(bool = true);

        void message(const char*, ...) const;
        void status(const char*, ...) const;
        __attribute__((noreturn)) void abort(errorcode_t) const;
        Reporter::String markup(markup_t, const char*, ...) const;
};
