//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
/**
 * @brief
 * String utils by SaturninTheAlien
 */
#include "PString.hpp"
#include <algorithm>
#include <iomanip>
#include <bitset>
#include <sstream>

namespace PString{

/**
 * Case mapping for the letters the game fonts can have: ASCII, Latin-1,
 * Latin Extended-A, Greek and Cyrillic. Anything else is left as it is.
 */
static char32_t lowercaseCodePoint(char32_t cp) {
    if(cp >= 'A' && cp <= 'Z') return cp + 0x20;
    if(cp < 0x80) return cp;

    // Latin-1: À..Þ except ×
    if(cp >= 0xC0 && cp <= 0xDE && cp != 0xD7) return cp + 0x20;
    if(cp == 0x178) return 0xFF;    // Ÿ
    if(cp == 0x1E9E) return 0xDF;   // capital sharp s

    // Latin Extended-A: upper/lower pairs, upper first
    if((cp >= 0x100 && cp <= 0x12F) || (cp >= 0x132 && cp <= 0x137) || (cp >= 0x14A && cp <= 0x177)){
        return (cp % 2 == 0) ? cp + 1 : cp;
    }
    if((cp >= 0x139 && cp <= 0x148) || (cp >= 0x179 && cp <= 0x17E)){
        return (cp % 2 == 1) ? cp + 1 : cp;
    }

    // Greek
    if(cp >= 0x391 && cp <= 0x3A9 && cp != 0x3A2) return cp + 0x20;
    if(cp == 0x386) return 0x3AC;
    if(cp >= 0x388 && cp <= 0x38A) return cp + 0x25;
    if(cp == 0x38C) return 0x3CC;
    if(cp == 0x38E || cp == 0x38F) return cp + 0x3F;

    // Cyrillic
    if(cp >= 0x410 && cp <= 0x42F) return cp + 0x20;
    if(cp >= 0x400 && cp <= 0x40F) return cp + 0x50;

    return cp;
}

static char32_t uppercaseCodePoint(char32_t cp) {
    if(cp >= 'a' && cp <= 'z') return cp - 0x20;
    if(cp < 0x80) return cp;

    // Latin-1: à..þ except ÷ (ß has no single-letter uppercase in common use)
    if(cp >= 0xE0 && cp <= 0xFE && cp != 0xF7) return cp - 0x20;
    if(cp == 0xFF) return 0x178;    // ÿ

    // Latin Extended-A
    if((cp >= 0x100 && cp <= 0x12F) || (cp >= 0x132 && cp <= 0x137) || (cp >= 0x14A && cp <= 0x177)){
        return (cp % 2 == 1) ? cp - 1 : cp;
    }
    if((cp >= 0x139 && cp <= 0x148) || (cp >= 0x179 && cp <= 0x17E)){
        return (cp % 2 == 0) ? cp - 1 : cp;
    }

    // Greek (final sigma becomes Σ)
    if(cp >= 0x3B1 && cp <= 0x3C9) return (cp == 0x3C2) ? 0x3A3 : cp - 0x20;
    if(cp == 0x3AC) return 0x386;
    if(cp >= 0x3AD && cp <= 0x3AF) return cp - 0x25;
    if(cp == 0x3CC) return 0x38C;
    if(cp == 0x3CD || cp == 0x3CE) return cp - 0x3F;

    // Cyrillic
    if(cp >= 0x430 && cp <= 0x44F) return cp - 0x20;
    if(cp >= 0x450 && cp <= 0x45F) return cp - 0x50;

    return cp;
}

/**
 * Decodes one UTF-8 sequence. Returns its length in bytes, or 0 if it is not
 * valid (then the caller copies the byte unchanged).
 */
static int decodeUTF8(const unsigned char* s, std::size_t available, char32_t& cp) {
    int len = 0;
    if (s[0] < 0x80) {
        cp = s[0];
        return 1;
    } else if ((s[0] & 0xE0) == 0xC0) {
        len = 2;
        cp = s[0] & 0x1F;
    } else if ((s[0] & 0xF0) == 0xE0) {
        len = 3;
        cp = s[0] & 0x0F;
    } else if ((s[0] & 0xF8) == 0xF0) {
        len = 4;
        cp = s[0] & 0x07;
    } else {
        return 0;
    }

    if (static_cast<std::size_t>(len) > available) {
        return 0;
    }
    for (int i = 1; i < len; ++i) {
        if ((s[i] & 0xC0) != 0x80) {
            return 0;
        }
        cp = (cp << 6) | (s[i] & 0x3F);
    }
    return len;
}

static void appendUTF8(std::string& out, char32_t cp) {
    auto byte = [&out](char32_t b) { out += static_cast<char>(b); };

    if (cp < 0x80) {
        byte(cp);
    } else if (cp < 0x800) {
        byte(0xC0 | (cp >> 6));
        byte(0x80 | (cp & 0x3F));
    } else if (cp < 0x10000) {
        byte(0xE0 | (cp >> 12));
        byte(0x80 | ((cp >> 6) & 0x3F));
        byte(0x80 | (cp & 0x3F));
    } else {
        byte(0xF0 | (cp >> 18));
        byte(0x80 | ((cp >> 12) & 0x3F));
        byte(0x80 | ((cp >> 6) & 0x3F));
        byte(0x80 | (cp & 0x3F));
    }
}

std::string lowercase(const std::string& src) {
    std::string str;
    str.reserve(src.size());

    const unsigned char* s = reinterpret_cast<const unsigned char*>(src.data());
    std::size_t i = 0;
    while (i < src.size()) {
        char32_t cp = 0;
        int len = decodeUTF8(s + i, src.size() - i, cp);
        if (len == 0) {
            str += src[i]; // not valid UTF-8, keep the byte
            ++i;
        } else {
            appendUTF8(str, lowercaseCodePoint(cp));
            i += len;
        }
    }

    return str;
}

std::string rtrim(const std::string & src){
    std::size_t i = src.size();
    while(i>0){
        --i;
        if(!std::isspace(src[i]))break;
    }

    ++i;
    return src.substr(0, i);
}

std::string unwindowsPath(const std::string& path){
    std::string res = path;
    std::size_t n = path.size();
    for(std::size_t i=0;i<n;++i){
        if(res[i]=='\\'){
            res[i]='/';
        }
    }
    return res;
}



bool startsWith(const std::string& str, const std::string& prefix){

    std::size_t n = prefix.size();
    if(str.size() < n)return false;

    return str.substr(0, n)==prefix;
}


bool endsWith(const std::string& str, const std::string& suffix){
    std::size_t n = suffix.size();
    if(str.size() < n) return false;
    return str.substr(str.size() - n, n)==suffix;
}


std::string removeSuffix(const std::string& str, const std::string& suffix){
    if(endsWith(str, suffix)){
        return str.substr(0, str.size() - suffix.size());
    }
    else{
        return str;
    }
}


static int getBytesNumber(char c){

    //ASCII ch
    if((c & 0x80)==0){
        return 1;
    }
    // 11110000

    //3 following bytes
    else if((c & 0xF0)==0xF0){
        return 4;
    }

    //2 following bytes
    else if((c & 0xE0)==0xE0){
        return 3;
    }

    //1 following bytes
    else if((c & 0xC0)==0xC0){
        return 2;
    }


    std::ostringstream os;
    os<<"Not allowed UTF-8 leading byte: "<<std::bitset<8>(c)<<std::endl;
    throw std::runtime_error(os.str());
}

const char* UTF8_Char::read(const char *str){
	for(int i=0;i<4;++i){
		this->data[i] = '\0';
	}

	int bytes = getBytesNumber(*str);
    char* data_ptr = this->data;
    if(bytes==1){
        *data_ptr = *str;
        ++str;
    }
    else{
        for(int i=0;i<bytes;++i){
            //if(*str=='\0')return str;
            // Not a correct following byte, it also works when *str=='\0'
            if( ((*str) & 0x80) == 0)return str;

            *data_ptr = *str;

            ++data_ptr;
            ++str;
        }
    }

	return str;
}

std::optional<char32_t> UTF8_Char::codePoint() const {
    const unsigned char* s = reinterpret_cast<const unsigned char*>(this->data);
    std::size_t n = std::char_traits<char>::length(this->data);

    char32_t cp = 0;
    if (n == 0 || decodeUTF8(s, n, cp) == 0) {
        return std::nullopt;
    }
    return cp;
}

static UTF8_Char mapCase(UTF8_Char src, char32_t (*map)(char32_t)) {
    std::optional<char32_t> cp = src.codePoint();
    if (!cp) {
        return src;
    }

    std::string mapped;
    appendUTF8(mapped, map(*cp));

    UTF8_Char res;
    res.read(mapped.c_str());
    return res;
}

UTF8_Char lowercase(UTF8_Char src){
    return mapCase(src, lowercaseCodePoint);
}

UTF8_Char uppercase(UTF8_Char src){
    return mapCase(src, uppercaseCodePoint);
}

}