/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "WebUtils.h"
#include "DomElement.h"
#include "rapidxml/rapidxml.hpp"
#include "Wt/WException"
#include "Wt/WString"
#include "Wt/Utils"

#include <boost/algorithm/string.hpp>
#include <boost/version.hpp>

#include <ctype.h>
#include <stdio.h>
#include <fstream>

#ifdef WIN32
#include <windows.h>
#define snprintf _snprintf
#else
#include <stdlib.h>
#endif // WIN32

#if !defined(WT_NO_SPIRIT) && BOOST_VERSION >= 104700
#  define SPIRIT_FLOAT_FORMAT
#endif

#ifdef SPIRIT_FLOAT_FORMAT
#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/karma.hpp>
#else
#include <boost/math/special_functions/fpclassify.hpp>
#include <boost/math/special_functions/sign.hpp>
#endif // SPIRIT_FLOAT_FORMAT

namespace Wt {
  namespace Utils {

std::string append(const std::string& s, char c)
{
  if (s.empty() || s[s.length() - 1] != c)
    return s + c;
  else
    return s;
}

std::string prepend(const std::string& s, char c)
{
  if (s.empty() || s[0] != c)
    return c + s;
  else
    return s;
}

std::string& replace(std::string& s, char c, const std::string& r)
{
  std::string::size_type p = 0;

  while ((p = s.find(c, p)) != std::string::npos) {
    s.replace(p, 1, r);
    p += r.length();
  }

  return s;
}

std::string& replace(std::string& s, const std::string& k, const std::string& r)
{
  std::string::size_type p = 0;

  while ((p = s.find(k, p)) != std::string::npos) {
    s.replace(p, k.length(), r);
    p += r.length();
  }

  return s;
}

std::string lowerCase(const std::string& s)
{
  std::string result = s;
  for (unsigned i = 0; i < result.length(); ++i)
    result[i] = tolower(result[i]);
  return result;
}

void sanitizeUnicode(EscapeOStream& sout, const std::string& text)
{
  char buf[4];

  for (const char *c = text.c_str(); *c;) {
    char *b = buf;
    // but copy_check_utf8() does not declare the following ranges illegal:
    //  U+D800-U+DFFF
    //  U+FFFE-U+FFFF
    rapidxml::xml_document<>::copy_check_utf8(c, b);
    for (char *i = buf; i < b; ++i)
      sout << *i;
  }
}

std::string eraseWord(const std::string& s, const std::string& w)
{
  std::string ss = s;
  std::string::size_type p;

  if ((p = ss.find(w)) != std::string::npos) {
    ss.erase(ss.begin() + p, ss.begin() + p + w.length());
    if (p > 1) {
      if (ss[p-1] == ' ')
	ss.erase(ss.begin() + (p - 1));
    } else
      if (p < ss.length() && ss[p] == ' ')
	ss.erase(ss.begin() + p);
  }

  return ss;
}

std::string addWord(const std::string& s, const std::string& w)
{
  if (s.empty())
    return w;
  else
    return s + ' ' + w;
}

char *itoa(int value, char *result, int base) {
  char* out = result;
  int quotient = value;

  if (quotient < 0)
    quotient = -quotient;

  do {
    *out =
      "0123456789abcdefghijklmnopqrstuvwxyz"[quotient % base];
    ++out;
    quotient /= base;
  } while (quotient);

  if (value < 0 && base == 10)
    *out++ = '-';

  std::reverse(result, out);
  *out = 0;

  return result;
}

char *lltoa(long long value, char *result, int base) {
  char* out = result;
  long long quotient = value;

  if (quotient < 0)
    quotient = -quotient;

  do {
    *out =
      "0123456789abcdefghijklmnopqrstuvwxyz"[ quotient % base ];
    ++out;
    quotient /= base;
  } while (quotient);

  if (value < 0 && base == 10)
    *out++ = '-';
  std::reverse(result, out);
  *out = 0;

  return result;
}

char *pad_itoa(int value, int length, char *result) {
  static const int exp[] = { 1, 10, 100, 1000, 10000, 100000, 100000 };

  result[length] = 0;

  for (int i = 0; i < length; ++i) {
    int b = exp[length - i - 1];
    if (value >= b)
      result[i] = '0' + (value / b) % 10;
    else
      result[i] = '0';
  }

  return result;
}

#ifdef SPIRIT_FLOAT_FORMAT
namespace {
  using namespace boost::spirit;
  using namespace boost::spirit::karma;

  // adjust rendering for JS flaots
  template <typename T>
  struct JavaScriptPolicy : karma::real_policies<T>
  {
    // not 'nan', but 'NaN'
    template <typename CharEncoding, typename Tag, typename OutputIterator>
    static bool nan (OutputIterator& sink, T n, bool force_sign)
    {
      return string_inserter<CharEncoding, Tag>::call(sink, "NaN");
    }

    // not 'inf', but 'Infinity'
    template <typename CharEncoding, typename Tag, typename OutputIterator>
    static bool inf (OutputIterator& sink, T n, bool force_sign)
    {
      return sign_inserter::call(sink, false, (n<0), force_sign) &&
        string_inserter<CharEncoding, Tag>::call(sink, "Infinity");
    }

    static int floatfield(T t) {
      return (t != 0.0) && ((t < 0.001) || (t > 1E8)) ?
	karma::real_policies<T>::fmtflags::scientific :
	karma::real_policies<T>::fmtflags::fixed;
    }

    // 7 significant numbers; about float precision
    static unsigned precision(T) { return 7; }
  };

  typedef real_generator<double, JavaScriptPolicy<double> >
    KarmaJavaScriptDouble;
}

static inline char *generic_double_to_str(double d, char *buf)
{
  using namespace boost::spirit;
  using namespace boost::spirit::karma;
  char *p = buf;
  if (d != 0)
    generate(p, KarmaJavaScriptDouble(), d);
  else
    *p++ = '0';
  *p = '\0';
  return buf;
}
#else
static inline char *generic_double_to_str(double d, char *buf)
{
  if (!boost::math::isnan(d)) {
    if (!boost::math::isinf(d)) {
      sprintf(buf, "%.7e", d);
    } else {
      if (d > 0) {
        sprintf(buf, "Infinity");
      } else {
        sprintf(buf, "-Infinity");
      }
    }
  } else {
    sprintf(buf, "NaN");
  }
  return buf;
}
#endif

char *round_str(double d, int digits, char *buf) {
#ifdef SPIRIT_FLOAT_FORMAT
  return generic_double_to_str(d, buf);
#else
  if (((d == 0)
       || (d > 1 && d < 1000000)
       || (d < -1 && d > -1000000))
      && digits < 7) {
    // range where a very fast float->string converter works
    // mainly intended to render floats for 2D drawing canvas
    static const int exp[] = { 1, 10, 100, 1000, 10000, 100000, 1000000 };

    long long i = static_cast<long long>(d * exp[digits] + (d > 0 ? 0.49 : -0.49));
    lltoa(i, buf);
    char *num = buf;

    if (num[0] == '-')
      ++num;
    int len = std::strlen(num);

    if (len <= digits) {
      int shift = digits + 1 - len;
      for (int i = digits + 1; i >= 0; --i) {
        if (i >= shift)
          num[i] = num[i - shift];
        else
          num[i] = '0';
      }
      len = digits + 1;
    }
    int dotPos = (std::max)(len - digits, 0);
    for (int i = digits + 1; i >= 0; --i)
      num[dotPos + i + 1] = num[dotPos + i];
    num[dotPos] = '.';
    return buf;
  } else {
    // an implementation that covers everything
    return generic_double_to_str(d, buf);
  }
#endif
}

std::string urlEncode(const std::string& url, const std::string& allowed)
{
  return DomElement::urlEncodeS(url, allowed);
}

std::string dataUrlDecode(const std::string& url,
			  std::vector<unsigned char> &data)
{
  return std::string();
}

void inplaceUrlDecode(std::string &text)
{
  // Note: there is a Java-too duplicate of this function in Wt/Utils.C
  std::size_t j = 0;

  for (std::size_t i = 0; i < text.length(); ++i) {
    char c = text[i];

    if (c == '+') {
      text[j++] = ' ';
    } else if (c == '%' && i + 2 < text.length()) {
      std::string h = text.substr(i + 1, 2);
      char *e = 0;
      int hval = std::strtol(h.c_str(), &e, 16);

      if (*e == 0) {
	text[j++] = (char)hval;
	i += 2;
      } else {
	// not a proper %XX with XX hexadecimal format
	text[j++] = c;
      }
    } else
      text[j++] = c;
  }

  text.erase(j);
}

void split(SplitSet& tokens, const std::string &in, const char *sep,
	   bool compress_adjacent_tokens)
{
    boost::split(tokens, in, boost::is_any_of(sep),
		 compress_adjacent_tokens?
		 boost::algorithm::token_compress_on:
		 boost::algorithm::token_compress_off);
}

std::string EncodeHttpHeaderField(const std::string &fieldname,
                                  const WString &fieldValue)
{
  // This implements RFC 5987
  return fieldname + "*=UTF-8''" + urlEncode(fieldValue.toUTF8());
}

std::string readFile(const std::string& fname) 
{
  std::ifstream f(fname.c_str(), std::ios::in | std::ios::binary);
  
  if (!f)
    throw WException("Could not load " + fname);
  
  f.seekg(0, std::ios::end);
  int length = f.tellg();
  f.seekg(0, std::ios::beg);
  
  boost::scoped_array<char> ftext(new char[length + 1]);
  f.read(ftext.get(), length);
  ftext[length] = 0;

  return std::string(ftext.get());
}

WString formatFloat(const WString &format, double value)
{
  std::string f = format.toUTF8();
  int buflen = f.length() + 15;

  char *buf = new char[buflen];

  snprintf(buf, buflen, f.c_str(), value);
  buf[buflen - 1] = 0;

  WString result = WT_USTRING::fromUTF8(buf);

  delete[] buf;

  return result;

}

std::string splitEntryToString(SplitEntry se)
{
#ifndef WT_TARGET_JAVA
  return std::string(se.begin(), se.end());
#else
  return se;
#endif
}
  
  }
}
