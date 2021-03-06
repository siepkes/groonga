/* -*- c-basic-offset: 2 -*- */
/*
  Copyright(C) 2012-2018 Brazil

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License version 2.1 as published by the Free Software Foundation.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <string.h>

#include "grn_normalizer.h"
#include "grn_string.h"
#include "grn_nfkc.h"
#include <groonga/normalizer.h>
#include <groonga/tokenizer.h>

grn_rc
grn_normalizer_register(grn_ctx *ctx,
                        const char *name_ptr,
                        int name_length,
                        grn_proc_func *init,
                        grn_proc_func *next,
                        grn_proc_func *fin)
{
  grn_expr_var vars[] = {
    { NULL, 0 }
  };
  GRN_PTR_INIT(&vars[0].value, 0, GRN_ID_NIL);

  if (name_length < 0) {
    name_length = strlen(name_ptr);
  }

  {
    grn_obj * const normalizer = grn_proc_create(ctx,
                                                 name_ptr, name_length,
                                                 GRN_PROC_NORMALIZER,
                                                 init, next, fin,
                                                 sizeof(*vars) / sizeof(vars),
                                                 vars);
    if (!normalizer) {
      GRN_PLUGIN_ERROR(ctx, GRN_NORMALIZER_ERROR,
                       "[normalizer] failed to register normalizer: <%.*s>",
                       name_length, name_ptr);
      return ctx->rc;
    }
  }
  return GRN_SUCCESS;
}

grn_rc
grn_normalizer_init(void)
{
  return GRN_SUCCESS;
}

grn_rc
grn_normalizer_fin(void)
{
  return GRN_SUCCESS;
}

static unsigned char symbol[] = {
  ',', '.', 0, ':', ';', '?', '!', 0, 0, 0, '`', 0, '^', '~', '_', 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, '-', '-', '/', '\\', 0, 0, '|', 0, 0, 0, '\'', 0,
  '"', '(', ')', 0, 0, '[', ']', '{', '}', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  '+', '-', 0, 0, 0, '=', 0, '<', '>', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  '$', 0, 0, '%', '#', '&', '*', '@', 0, 0, 0, 0, 0, 0, 0, 0
};

grn_inline static grn_obj *
eucjp_normalize(grn_ctx *ctx, grn_string *nstr)
{
  static uint16_t hankana[] = {
    0xa1a1, 0xa1a3, 0xa1d6, 0xa1d7, 0xa1a2, 0xa1a6, 0xa5f2, 0xa5a1, 0xa5a3,
    0xa5a5, 0xa5a7, 0xa5a9, 0xa5e3, 0xa5e5, 0xa5e7, 0xa5c3, 0xa1bc, 0xa5a2,
    0xa5a4, 0xa5a6, 0xa5a8, 0xa5aa, 0xa5ab, 0xa5ad, 0xa5af, 0xa5b1, 0xa5b3,
    0xa5b5, 0xa5b7, 0xa5b9, 0xa5bb, 0xa5bd, 0xa5bf, 0xa5c1, 0xa5c4, 0xa5c6,
    0xa5c8, 0xa5ca, 0xa5cb, 0xa5cc, 0xa5cd, 0xa5ce, 0xa5cf, 0xa5d2, 0xa5d5,
    0xa5d8, 0xa5db, 0xa5de, 0xa5df, 0xa5e0, 0xa5e1, 0xa5e2, 0xa5e4, 0xa5e6,
    0xa5e8, 0xa5e9, 0xa5ea, 0xa5eb, 0xa5ec, 0xa5ed, 0xa5ef, 0xa5f3, 0xa1ab,
    0xa1eb
  };
  static unsigned char dakuten[] = {
    0xf4, 0, 0, 0, 0, 0xac, 0, 0xae, 0, 0xb0, 0, 0xb2, 0, 0xb4, 0, 0xb6, 0,
    0xb8, 0, 0xba, 0, 0xbc, 0, 0xbe, 0, 0xc0, 0, 0xc2, 0, 0, 0xc5, 0, 0xc7,
    0, 0xc9, 0, 0, 0, 0, 0, 0, 0xd0, 0, 0, 0xd3, 0, 0, 0xd6, 0, 0, 0xd9, 0,
    0, 0xdc
  };
  static unsigned char handaku[] = {
    0xd1, 0, 0, 0xd4, 0, 0, 0xd7, 0, 0, 0xda, 0, 0, 0xdd
  };
  int16_t *ch;
  const unsigned char *s, *s_, *e;
  unsigned char *d, *d0, *d_, b;
  uint_least8_t *cp, *ctypes, ctype;
  size_t size = nstr->original_length_in_bytes, length = 0;
  int removeblankp = nstr->flags & GRN_STRING_REMOVE_BLANK;
  if (!(nstr->normalized = GRN_MALLOC(size * 2 + 1))) {
    ERR(GRN_NO_MEMORY_AVAILABLE,
        "[string][eucjp] failed to allocate normalized text space");
    return NULL;
  }
  d0 = (unsigned char *) nstr->normalized;
  if (nstr->flags & GRN_STRING_WITH_CHECKS) {
    if (!(nstr->checks = GRN_MALLOC(size * 2 * sizeof(int16_t) + 1))) {
      GRN_FREE(nstr->normalized);
      nstr->normalized = NULL;
      ERR(GRN_NO_MEMORY_AVAILABLE,
          "[string][eucjp] failed to allocate checks space");
      return NULL;
    }
  }
  ch = nstr->checks;
  if (nstr->flags & GRN_STRING_WITH_TYPES) {
    if (!(nstr->ctypes = GRN_MALLOC(size + 1))) {
      GRN_FREE(nstr->checks);
      GRN_FREE(nstr->normalized);
      nstr->checks = NULL;
      nstr->normalized = NULL;
      ERR(GRN_NO_MEMORY_AVAILABLE,
          "[string][eucjp] failed to allocate character types space");
      return NULL;
    }
  }
  cp = ctypes = nstr->ctypes;
  e = (unsigned char *)nstr->original + size;
  for (s = s_ = (unsigned char *) nstr->original, d = d_ = d0; s < e; s++) {
    if ((*s & 0x80)) {
      if (((s + 1) < e) && (*(s + 1) & 0x80)) {
        unsigned char c1 = *s++, c2 = *s, c3 = 0;
        switch (c1 >> 4) {
        case 0x08 :
          if (c1 == 0x8e && 0xa0 <= c2 && c2 <= 0xdf) {
            uint16_t c = hankana[c2 - 0xa0];
            switch (c) {
            case 0xa1ab :
              if (d > d0 + 1 && d[-2] == 0xa5
                  && 0xa6 <= d[-1] && d[-1] <= 0xdb && (b = dakuten[d[-1] - 0xa6])) {
                *(d - 1) = b;
                if (ch) { ch[-1] += 2; s_ += 2; }
                continue;
              } else {
                *d++ = c >> 8; *d = c & 0xff;
              }
              break;
            case 0xa1eb :
              if (d > d0 + 1 && d[-2] == 0xa5
                  && 0xcf <= d[-1] && d[-1] <= 0xdb && (b = handaku[d[-1] - 0xcf])) {
                *(d - 1) = b;
                if (ch) { ch[-1] += 2; s_ += 2; }
                continue;
              } else {
                *d++ = c >> 8; *d = c & 0xff;
              }
              break;
            default :
              *d++ = c >> 8; *d = c & 0xff;
              break;
            }
            ctype = GRN_CHAR_KATAKANA;
          } else {
            *d++ = c1; *d = c2;
            ctype = GRN_CHAR_OTHERS;
          }
          break;
        case 0x09 :
          *d++ = c1; *d = c2;
          ctype = GRN_CHAR_OTHERS;
          break;
        case 0x0a :
          switch (c1 & 0x0f) {
          case 1 :
            switch (c2) {
            case 0xbc :
              *d++ = c1; *d = c2;
              ctype = GRN_CHAR_KATAKANA;
              break;
            case 0xb9 :
              *d++ = c1; *d = c2;
              ctype = GRN_CHAR_KANJI;
              break;
            case 0xa1 :
              if (removeblankp) {
                if (cp > ctypes) { *(cp - 1) |= GRN_CHAR_BLANK; }
                continue;
              } else {
                *d = ' ';
                ctype = GRN_CHAR_BLANK|GRN_CHAR_SYMBOL;
              }
              break;
            default :
              if (c2 >= 0xa4 && (c3 = symbol[c2 - 0xa4])) {
                *d = c3;
                ctype = GRN_CHAR_SYMBOL;
              } else {
                *d++ = c1; *d = c2;
                ctype = GRN_CHAR_OTHERS;
              }
              break;
            }
            break;
          case 2 :
            *d++ = c1; *d = c2;
            ctype = GRN_CHAR_SYMBOL;
            break;
          case 3 :
            c3 = c2 - 0x80;
            if ('a' <= c3 && c3 <= 'z') {
              ctype = GRN_CHAR_ALPHA;
              *d = c3;
            } else if ('A' <= c3 && c3 <= 'Z') {
              ctype = GRN_CHAR_ALPHA;
              *d = c3 + 0x20;
            } else if ('0' <= c3 && c3 <= '9') {
              ctype = GRN_CHAR_DIGIT;
              *d = c3;
            } else {
              ctype = GRN_CHAR_OTHERS;
              *d++ = c1; *d = c2;
            }
            break;
          case 4 :
            *d++ = c1; *d = c2;
            ctype = GRN_CHAR_HIRAGANA;
            break;
          case 5 :
            *d++ = c1; *d = c2;
            ctype = GRN_CHAR_KATAKANA;
            break;
          case 6 :
          case 7 :
          case 8 :
            *d++ = c1; *d = c2;
            ctype = GRN_CHAR_SYMBOL;
            break;
          default :
            *d++ = c1; *d = c2;
            ctype = GRN_CHAR_OTHERS;
            break;
          }
          break;
        default :
          *d++ = c1; *d = c2;
          ctype = GRN_CHAR_KANJI;
          break;
        }
      } else {
        /* skip invalid character */
        continue;
      }
    } else {
      unsigned char c = *s;
      switch (c >> 4) {
      case 0 :
      case 1 :
        /* skip unprintable ascii */
        if (cp > ctypes) { *(cp - 1) |= GRN_CHAR_BLANK; }
        continue;
      case 2 :
        if (c == 0x20) {
          if (removeblankp) {
            if (cp > ctypes) { *(cp - 1) |= GRN_CHAR_BLANK; }
            continue;
          } else {
            *d = ' ';
            ctype = GRN_CHAR_BLANK|GRN_CHAR_SYMBOL;
          }
        } else {
          *d = c;
          ctype = GRN_CHAR_SYMBOL;
        }
        break;
      case 3 :
        *d = c;
        ctype = (c <= 0x39) ? GRN_CHAR_DIGIT : GRN_CHAR_SYMBOL;
        break;
      case 4 :
        *d = ('A' <= c) ? c + 0x20 : c;
        ctype = (c == 0x40) ? GRN_CHAR_SYMBOL : GRN_CHAR_ALPHA;
        break;
      case 5 :
        *d = (c <= 'Z') ? c + 0x20 : c;
        ctype = (c <= 0x5a) ? GRN_CHAR_ALPHA : GRN_CHAR_SYMBOL;
        break;
      case 6 :
        *d = c;
        ctype = (c == 0x60) ? GRN_CHAR_SYMBOL : GRN_CHAR_ALPHA;
        break;
      case 7 :
        *d = c;
        ctype = (c <= 0x7a) ? GRN_CHAR_ALPHA : (c == 0x7f ? GRN_CHAR_OTHERS : GRN_CHAR_SYMBOL);
        break;
      default :
        *d = c;
        ctype = GRN_CHAR_OTHERS;
        break;
      }
    }
    d++;
    length++;
    if (cp) { *cp++ = ctype; }
    if (ch) {
      *ch++ = (int16_t)(s + 1 - s_);
      s_ = s + 1;
      while (++d_ < d) { *ch++ = 0; }
    }
  }
  if (cp) { *cp = GRN_CHAR_NULL; }
  *d = '\0';
  nstr->n_characters = length;
  nstr->normalized_length_in_bytes = (size_t)(d - (unsigned char *)nstr->normalized);
  return NULL;
}

grn_inline static grn_obj *
sjis_normalize(grn_ctx *ctx, grn_string *nstr)
{
  static uint16_t hankana[] = {
    0x8140, 0x8142, 0x8175, 0x8176, 0x8141, 0x8145, 0x8392, 0x8340, 0x8342,
    0x8344, 0x8346, 0x8348, 0x8383, 0x8385, 0x8387, 0x8362, 0x815b, 0x8341,
    0x8343, 0x8345, 0x8347, 0x8349, 0x834a, 0x834c, 0x834e, 0x8350, 0x8352,
    0x8354, 0x8356, 0x8358, 0x835a, 0x835c, 0x835e, 0x8360, 0x8363, 0x8365,
    0x8367, 0x8369, 0x836a, 0x836b, 0x836c, 0x836d, 0x836e, 0x8371, 0x8374,
    0x8377, 0x837a, 0x837d, 0x837e, 0x8380, 0x8381, 0x8382, 0x8384, 0x8386,
    0x8388, 0x8389, 0x838a, 0x838b, 0x838c, 0x838d, 0x838f, 0x8393, 0x814a,
    0x814b
  };
  static unsigned char dakuten[] = {
    0x94, 0, 0, 0, 0, 0x4b, 0, 0x4d, 0, 0x4f, 0, 0x51, 0, 0x53, 0, 0x55, 0,
    0x57, 0, 0x59, 0, 0x5b, 0, 0x5d, 0, 0x5f, 0, 0x61, 0, 0, 0x64, 0, 0x66,
    0, 0x68, 0, 0, 0, 0, 0, 0, 0x6f, 0, 0, 0x72, 0, 0, 0x75, 0, 0, 0x78, 0,
    0, 0x7b
  };
  static unsigned char handaku[] = {
    0x70, 0, 0, 0x73, 0, 0, 0x76, 0, 0, 0x79, 0, 0, 0x7c
  };
  int16_t *ch;
  const unsigned char *s, *s_;
  unsigned char *d, *d0, *d_, b, *e;
  uint_least8_t *cp, *ctypes, ctype;
  size_t size = nstr->original_length_in_bytes, length = 0;
  int removeblankp = nstr->flags & GRN_STRING_REMOVE_BLANK;
  if (!(nstr->normalized = GRN_MALLOC(size * 2 + 1))) {
    ERR(GRN_NO_MEMORY_AVAILABLE,
        "[string][sjis] failed to allocate normalized text space");
    return NULL;
  }
  d0 = (unsigned char *) nstr->normalized;
  if (nstr->flags & GRN_STRING_WITH_CHECKS) {
    if (!(nstr->checks = GRN_MALLOC(size * 2 * sizeof(int16_t) + 1))) {
      GRN_FREE(nstr->normalized);
      nstr->normalized = NULL;
      ERR(GRN_NO_MEMORY_AVAILABLE,
          "[string][sjis] failed to allocate checks space");
      return NULL;
    }
  }
  ch = nstr->checks;
  if (nstr->flags & GRN_STRING_WITH_TYPES) {
    if (!(nstr->ctypes = GRN_MALLOC(size + 1))) {
      GRN_FREE(nstr->checks);
      GRN_FREE(nstr->normalized);
      nstr->checks = NULL;
      nstr->normalized = NULL;
      ERR(GRN_NO_MEMORY_AVAILABLE,
          "[string][sjis] failed to allocate character types space");
      return NULL;
    }
  }
  cp = ctypes = nstr->ctypes;
  e = (unsigned char *)nstr->original + size;
  for (s = s_ = (unsigned char *) nstr->original, d = d_ = d0; s < e; s++) {
    if ((*s & 0x80)) {
      if (0xa0 <= *s && *s <= 0xdf) {
        uint16_t c = hankana[*s - 0xa0];
        switch (c) {
        case 0x814a :
          if (d > d0 + 1 && d[-2] == 0x83
              && 0x45 <= d[-1] && d[-1] <= 0x7a && (b = dakuten[d[-1] - 0x45])) {
            *(d - 1) = b;
            if (ch) { ch[-1]++; s_++; }
            continue;
          } else {
            *d++ = c >> 8; *d = c & 0xff;
          }
          break;
        case 0x814b :
          if (d > d0 + 1 && d[-2] == 0x83
              && 0x6e <= d[-1] && d[-1] <= 0x7a && (b = handaku[d[-1] - 0x6e])) {
            *(d - 1) = b;
            if (ch) { ch[-1]++; s_++; }
            continue;
          } else {
            *d++ = c >> 8; *d = c & 0xff;
          }
          break;
        default :
          *d++ = c >> 8; *d = c & 0xff;
          break;
        }
        ctype = GRN_CHAR_KATAKANA;
      } else {
        if ((s + 1) < e && 0x40 <= *(s + 1) && *(s + 1) <= 0xfc) {
          unsigned char c1 = *s++, c2 = *s, c3 = 0;
          if (0x81 <= c1 && c1 <= 0x87) {
            switch (c1 & 0x0f) {
            case 1 :
              switch (c2) {
              case 0x5b :
                *d++ = c1; *d = c2;
                ctype = GRN_CHAR_KATAKANA;
                break;
              case 0x58 :
                *d++ = c1; *d = c2;
                ctype = GRN_CHAR_KANJI;
                break;
              case 0x40 :
                if (removeblankp) {
                  if (cp > ctypes) { *(cp - 1) |= GRN_CHAR_BLANK; }
                  continue;
                } else {
                  *d = ' ';
                  ctype = GRN_CHAR_BLANK|GRN_CHAR_SYMBOL;
                }
                break;
              default :
                if (0x43 <= c2 && c2 <= 0x7e && (c3 = symbol[c2 - 0x43])) {
                  *d = c3;
                  ctype = GRN_CHAR_SYMBOL;
                } else if (0x7f <= c2 && c2 <= 0x97 && (c3 = symbol[c2 - 0x44])) {
                  *d = c3;
                  ctype = GRN_CHAR_SYMBOL;
                } else {
                  *d++ = c1; *d = c2;
                  ctype = GRN_CHAR_OTHERS;
                }
                break;
              }
              break;
            case 2 :
              c3 = c2 - 0x1f;
              if (0x4f <= c2 && c2 <= 0x58) {
                ctype = GRN_CHAR_DIGIT;
                *d = c2 - 0x1f;
              } else if (0x60 <= c2 && c2 <= 0x79) {
                ctype = GRN_CHAR_ALPHA;
                *d = c2 + 0x01;
              } else if (0x81 <= c2 && c2 <= 0x9a) {
                ctype = GRN_CHAR_ALPHA;
                *d = c2 - 0x20;
              } else if (0x9f <= c2 && c2 <= 0xf1) {
                *d++ = c1; *d = c2;
                ctype = GRN_CHAR_HIRAGANA;
              } else {
                *d++ = c1; *d = c2;
                ctype = GRN_CHAR_OTHERS;
              }
              break;
            case 3 :
              if (0x40 <= c2 && c2 <= 0x96) {
                *d++ = c1; *d = c2;
                ctype = GRN_CHAR_KATAKANA;
              } else {
                *d++ = c1; *d = c2;
                ctype = GRN_CHAR_SYMBOL;
              }
              break;
            case 4 :
            case 7 :
              *d++ = c1; *d = c2;
              ctype = GRN_CHAR_SYMBOL;
              break;
            default :
              *d++ = c1; *d = c2;
              ctype = GRN_CHAR_OTHERS;
              break;
            }
          } else {
            *d++ = c1; *d = c2;
            ctype = GRN_CHAR_KANJI;
          }
        } else {
          /* skip invalid character */
          continue;
        }
      }
    } else {
      unsigned char c = *s;
      switch (c >> 4) {
      case 0 :
      case 1 :
        /* skip unprintable ascii */
        if (cp > ctypes) { *(cp - 1) |= GRN_CHAR_BLANK; }
        continue;
      case 2 :
        if (c == 0x20) {
          if (removeblankp) {
            if (cp > ctypes) { *(cp - 1) |= GRN_CHAR_BLANK; }
            continue;
          } else {
            *d = ' ';
            ctype = GRN_CHAR_BLANK|GRN_CHAR_SYMBOL;
          }
        } else {
          *d = c;
          ctype = GRN_CHAR_SYMBOL;
        }
        break;
      case 3 :
        *d = c;
        ctype = (c <= 0x39) ? GRN_CHAR_DIGIT : GRN_CHAR_SYMBOL;
        break;
      case 4 :
        *d = ('A' <= c) ? c + 0x20 : c;
        ctype = (c == 0x40) ? GRN_CHAR_SYMBOL : GRN_CHAR_ALPHA;
        break;
      case 5 :
        *d = (c <= 'Z') ? c + 0x20 : c;
        ctype = (c <= 0x5a) ? GRN_CHAR_ALPHA : GRN_CHAR_SYMBOL;
        break;
      case 6 :
        *d = c;
        ctype = (c == 0x60) ? GRN_CHAR_SYMBOL : GRN_CHAR_ALPHA;
        break;
      case 7 :
        *d = c;
        ctype = (c <= 0x7a) ? GRN_CHAR_ALPHA : (c == 0x7f ? GRN_CHAR_OTHERS : GRN_CHAR_SYMBOL);
        break;
      default :
        *d = c;
        ctype = GRN_CHAR_OTHERS;
        break;
      }
    }
    d++;
    length++;
    if (cp) { *cp++ = ctype; }
    if (ch) {
      *ch++ = (int16_t)(s + 1 - s_);
      s_ = s + 1;
      while (++d_ < d) { *ch++ = 0; }
    }
  }
  if (cp) { *cp = GRN_CHAR_NULL; }
  *d = '\0';
  nstr->n_characters = length;
  nstr->normalized_length_in_bytes = (size_t)(d - (unsigned char *)nstr->normalized);
  return NULL;
}

#ifdef GRN_WITH_NFKC
static grn_inline int
grn_str_charlen_utf8(grn_ctx *ctx, const unsigned char *str, const unsigned char *end)
{
  /* MEMO: This function allows non-null-terminated string as str. */
  /*       But requires the end of string. */
  const unsigned char *p = str;
  if (end <= p || !*p) { return 0; }
  if (*p & 0x80) {
    int b, w;
    int size;
    int i;
    for (b = 0x40, w = 0; b && (*p & b); b >>= 1, w++);
    if (!w) {
      GRN_LOG(ctx, GRN_LOG_WARNING,
              "invalid utf8 string: the first bit is 0x80: <%.*s>: <%.*s>",
              (int)(end - p), p,
              (int)(end - str), str);
      return 0;
    }
    size = w + 1;
    for (i = 1; i < size; i++) {
      if (++p >= end) {
        GRN_LOG(ctx, GRN_LOG_WARNING,
                "invalid utf8 string: too short: "
                "%d byte is required but %d byte is given: <%.*s>",
                size, i,
                (int)(end - str), str);
        return 0;
      }
      if (!*p) {
        GRN_LOG(ctx, GRN_LOG_WARNING,
                "invalid utf8 string: NULL character is found: <%.*s>",
                (int)(end - str), str);
        return 0;
      }
      if ((*p & 0xc0) != 0x80) {
        GRN_LOG(ctx, GRN_LOG_WARNING,
                "invalid utf8 string: 0x80 is not allowed: <%.*s>: <%.*s>",
                (int)(end - p), p,
                (int)(end - str), str);
        return 0;
      }
    }
    return size;
  } else {
    return 1;
  }
  return 0;
}

typedef grn_char_type (*grn_nfkc_char_type_func)(const unsigned char *utf8);
typedef const char *(*grn_nfkc_decompose_func)(const unsigned char *utf8);
typedef const char *(*grn_nfkc_compose_func)(const unsigned char *prefix_utf8,
                                             const unsigned char *suffix_utf8);

typedef struct {
  grn_nfkc_char_type_func char_type_func;
  grn_nfkc_decompose_func decompose_func;
  grn_nfkc_compose_func compose_func;
  grn_bool include_removed_source_location;
  grn_bool report_source_offset;
  grn_bool unify_kana;
  grn_bool unify_kana_case;
  grn_bool unify_kana_voiced_sound_mark;
  grn_bool unify_hyphen;
  grn_bool unify_prolonged_sound_mark;
  grn_bool unify_hyphen_and_prolonged_sound_mark;
  grn_bool unify_middle_dot;
  grn_bool unify_katakana_v_sounds;
  grn_bool unify_katakana_bu_sound;
} grn_utf8_normalize_options;

static void
utf8_normalize_options_init(grn_utf8_normalize_options *options,
                            grn_nfkc_char_type_func char_type_func,
                            grn_nfkc_decompose_func decompose_func,
                            grn_nfkc_compose_func compose_func)
{
  options->char_type_func = char_type_func;
  options->decompose_func = decompose_func;
  options->compose_func = compose_func;
  options->include_removed_source_location = GRN_TRUE;
  options->report_source_offset = GRN_FALSE;
  options->unify_kana = GRN_FALSE;
  options->unify_kana_case = GRN_FALSE;
  options->unify_kana_voiced_sound_mark = GRN_FALSE;
  options->unify_hyphen = GRN_FALSE;
  options->unify_prolonged_sound_mark = GRN_FALSE;
  options->unify_hyphen_and_prolonged_sound_mark = GRN_FALSE;
  options->unify_middle_dot = GRN_FALSE;
  options->unify_katakana_v_sounds = GRN_FALSE;
  options->unify_katakana_bu_sound = GRN_FALSE;
}

grn_inline static const unsigned char *
utf8_normalize_unify_kana(const unsigned char *utf8_char,
                          unsigned char *unified)
{
  if (utf8_char[0] == 0xe3 &&
      /* U+30A1 KATAKANA LETTER SMALL A ..
       * U+30F6 KATAKANA LETTER SMALL KE
       *
       * U+30FD KATAKANA ITERATION MARK ..
       * U+30F6 KATAKANA LETTER SMALL KE */
      ((utf8_char[1] == 0x82 && 0xa1 <= utf8_char[2]) ||
       (utf8_char[1] == 0x83 && utf8_char[2] <= 0xb6) ||
       (utf8_char[1] == 0x83 && (0xbd <= utf8_char[2] &&
                                 utf8_char[2] <= 0xbe)))) {
    unified[0] = utf8_char[0];
    if (utf8_char[2] & 0x20) {
      unified[1] = utf8_char[1] - 1;
    } else {
      unified[1] = utf8_char[1] - 2;
    }
    unified[2] = utf8_char[2] ^ 0x20;
    return unified;
  }

  return utf8_char;
}

grn_inline static const unsigned char *
utf8_normalize_unify_hiragana_case(const unsigned char *utf8_char,
                                   unsigned char *unified)
{
  if (utf8_char[0] == 0xe3) {
    if ((utf8_char[1] == 0x81 && (0x81 <= utf8_char[2] &&
                                  utf8_char[2] <= 0x89)) ||
        (utf8_char[1] == 0x81 && utf8_char[2] == 0xa3) ||
        (utf8_char[1] == 0x82 && (0x83 <= utf8_char[2] &&
                                  utf8_char[2] <= 0x87))) {
      /* U+3041 HIRAGANA LETTER SMALL A ..
       * U+3049 HIRAGANA LETTER SMALL O
       *
       * U+3063 HIRAGANA LETTER SMALL TU
       *
       * U+3083 HIRAGANA LETTER SMALL YA ..
       * U+3087 HIRAGANA LETTER SMALL YO */
      if (utf8_char[2] & 0x1) {
        unified[0] = utf8_char[0];
        unified[1] = utf8_char[1];
        unified[2] = utf8_char[2] + 1;
        return unified;
      }
    } else if (utf8_char[1] == 0x82 && utf8_char[2] == 0x8e) {
      /* U+308E HIRAGANA LETTER SMALL WA */
      unified[0] = utf8_char[0];
      unified[1] = utf8_char[1];
      unified[2] = utf8_char[2] + 1;
      return unified;
    } else if (utf8_char[1] == 0x82 && utf8_char[2] == 0x95) {
      /* U+3095 HIRAGANA LETTER SMALL KA */
      unified[0] = utf8_char[0];
      unified[1] = 0x81;
      unified[2] = 0x8b;
      return unified;
    } else if (utf8_char[1] == 0x82 && utf8_char[2] == 0x96) {
      /* U+3096 HIRAGANA LETTER SMALL KE */
      unified[0] = utf8_char[0];
      unified[1] = 0x81;
      unified[2] = 0x91;
      return unified;
    }
  }

  return utf8_char;
}

grn_inline static const unsigned char *
utf8_normalize_unify_katakana_case(const unsigned char *utf8_char,
                                   unsigned char *unified)
{
  if (utf8_char[0] == 0xe3) {
    if ((utf8_char[1] == 0x82 && (0xa1 <= utf8_char[2] &&
                                  utf8_char[2] <= 0xa9)) ||
        (utf8_char[1] == 0x83 && utf8_char[2] == 0x83) ||
        (utf8_char[1] == 0x83 && (0xa3 <= utf8_char[2] &&
                                  utf8_char[2] <= 0xa7))) {
      /* U+30A1 KATAKANA LETTER SMALL A ..
       * U+30A9 KATAKANA LETTER SMALL O
       *
       * U+30C3 KATAKANA LETTER SMALL TU
       *
       * U+30E3 KATAKANA LETTER SMALL YA ..
       * U+30E7 KATAKANA LETTER SMALL YO */
      if (utf8_char[2] & 0x1) {
        unified[0] = utf8_char[0];
        unified[1] = utf8_char[1];
        unified[2] = utf8_char[2] + 1;
        return unified;
      }
    } else if (utf8_char[1] == 0x83 && utf8_char[2] == 0xae) {
      /* U+30EE KATAKANA LETTER SMALL WA */
      unified[0] = utf8_char[0];
      unified[1] = utf8_char[1];
      unified[2] = utf8_char[2] + 1;
      return unified;
    } else if (utf8_char[1] == 0x83 && utf8_char[2] == 0xb5) {
      /* U+3095 HIRAGANA LETTER SMALL KA */
      unified[0] = utf8_char[0];
      unified[1] = 0x82;
      unified[2] = 0xab;
      return unified;
    } else if (utf8_char[1] == 0x83 && utf8_char[2] == 0xb6) {
      /* U+3096 HIRAGANA LETTER SMALL KE */
      unified[0] = utf8_char[0];
      unified[1] = 0x82;
      unified[2] = 0xb1;
      return unified;
    }
  }

  return utf8_char;
}

grn_inline static const unsigned char *
utf8_normalize_unify_hiragana_voiced_sound_mark(const unsigned char *utf8_char,
                                                unsigned char *unified)
{
  if (utf8_char[0] == 0xe3) {
    if ((utf8_char[1] == 0x81 && (0x8c <= utf8_char[2] &&
                                  utf8_char[2] <= 0xa2))) {
      /* U+304C HIRAGANA LETTER GA ..
       * U+3062 HIRAGANA LETTER DI */
      if (!(utf8_char[2] & 0x1)) {
        unified[0] = utf8_char[0];
        unified[1] = utf8_char[1];
        unified[2] = utf8_char[2] - 1;
        return unified;
      }
    } else if ((utf8_char[1] == 0x81 && (0xa5 <= utf8_char[2] &&
                                         utf8_char[2] <= 0xa9))) {
      /* U+3065 HIRAGANA LETTER DU ..
       * U+3069 HIRAGANA LETTER DO */
      if (utf8_char[2] & 0x1) {
        unified[0] = utf8_char[0];
        unified[1] = utf8_char[1];
        unified[2] = utf8_char[2] - 1;
        return unified;
      }
    } else if ((utf8_char[1] == 0x81 && (0xb0 <= utf8_char[2] &&
                                         utf8_char[2] <= 0xbd))) {
      /* U+3070 HIRAGANA LETTER BA ..
       * U+307D HIRAGANA LETTER PO */
      unsigned char mod3 = (utf8_char[2] - 1) % 3;
      if (mod3 != 0) {
        unified[0] = utf8_char[0];
        unified[1] = utf8_char[1];
        unified[2] = utf8_char[2] - mod3;
        return unified;
      }
    }
  }

  return utf8_char;
}

grn_inline static const unsigned char *
utf8_normalize_unify_katakana_voiced_sound_mark(const unsigned char *utf8_char,
                                                unsigned char *unified)
{
  if (utf8_char[0] == 0xe3) {
    if (utf8_char[1] == 0x83 && utf8_char[2] == 0x80) {
      /* U+30C0 KATAKANA LETTER DA */
      unified[0] = utf8_char[0];
      unified[1] = 0x82;
      unified[2] = 0xbf;
      return unified;
    } else if ((utf8_char[1] == 0x82 && 0xac <= utf8_char[2]) ||
               (utf8_char[1] == 0x83 && utf8_char[2] <= 0x82)) {
      /* U+30AC KATAKANA LETTER GA ..
       * U+30C2 KATAKANA LETTER DI */
      if (!(utf8_char[2] & 0x1)) {
        unified[0] = utf8_char[0];
        unified[1] = utf8_char[1];
        unified[2] = utf8_char[2] - 1;
        return unified;
      }
    } else if ((utf8_char[1] == 0x83 && (0x85 <= utf8_char[2] &&
                                         utf8_char[2] <= 0x89))) {
      /* U+30C5 KATAKANA LETTER DU ..
       * U+30C9 KATAKANA LETTER DO */
      if (utf8_char[2] & 0x1) {
        unified[0] = utf8_char[0];
        unified[1] = utf8_char[1];
        unified[2] = utf8_char[2] - 1;
        return unified;
      }
    } else if ((utf8_char[1] == 0x83 && (0x90 <= utf8_char[2] &&
                                         utf8_char[2] <= 0x9d))) {
      /* U+30D0 KATAKANA LETTER BA ..
       * U+30DD KATAKANA LETTER PO */
      unsigned char mod3 = (utf8_char[2] - 2) % 3;
      if (mod3 != 0) {
        unified[0] = utf8_char[0];
        unified[1] = utf8_char[1];
        unified[2] = utf8_char[2] - mod3;
        return unified;
      }
    }
  }

  return utf8_char;
}

grn_inline static const grn_bool
utf8_normalize_is_hyphen_famity(const unsigned char *utf8_char,
                                size_t length)
{
  if (length == 1) {
    if (utf8_char[0] == '-') {
      /* U+002D HYPHEN-MINUS */
      return GRN_TRUE;
    }
  } else if (length == 2) {
    switch (utf8_char[0]) {
    case 0xcb :
      if (utf8_char[1] == 0x97) {
        /* U+02D7 MODIFIER LETTER MINUS SIGN */
        return GRN_TRUE;
      }
      break;
    case 0xd6 :
      if (utf8_char[1] == 0x8a) {
        /* U+058A ARMENIAN HYPHEN */
        return GRN_TRUE;
      }
      break;
    default :
      break;
    }
  } else if (length == 3) {
    if (utf8_char[0] == 0xe2) {
      if (utf8_char[1] == 0x80 &&
          (0x90 <= utf8_char[2] && utf8_char[2] <= 0x93)) {
        /* U+2010 HYPHEN ..
         * U+2013 EN DASH */
        return GRN_TRUE;
      } else if (utf8_char[1] == 0x81 &&
                 (utf8_char[2] == 0x83 ||
                  utf8_char[2] == 0xbb)) {
        /* U+2043 HYPHEN BULLET */
        /* U+207B SUPERSCRIPT MINUS */
        return GRN_TRUE;
      } else if (utf8_char[1] == 0x82 && utf8_char[2] == 0x8b) {
        /* U+208B SUBSCRIPT MINUS */
        return GRN_TRUE;
      } else if (utf8_char[1] == 0x88 && utf8_char[2] == 0x92) {
        /* U+2212 MINUS SIGN */
        return GRN_TRUE;
      }
    }
  }

  return GRN_FALSE;
}

grn_inline static const grn_bool
utf8_normalize_is_prolonged_sound_mark_famity(const unsigned char *utf8_char,
                                              size_t length)
{
  if (length == 3) {
    if (utf8_char[0] == 0xe2) {
      if (utf8_char[1] == 0x80 &&
          (0x94 <= utf8_char[2] && utf8_char[2] <= 0x95)) {
        /* U+2014 EM DASH ..
         * U+2015 HORIZONTAL BAR */
        return GRN_TRUE;
      } else if (utf8_char[1] == 0x94 &&
          (0x80 <= utf8_char[2] && utf8_char[2] <= 0x81)) {
        /* U+2500 BOX DRAWINGS LIGHT HORIZONTAL ..
         * U+2501 BOX DRAWINGS HEAVY HORIZONTAL */
        return GRN_TRUE;
      }
    } else if (utf8_char[0] == 0xe3) {
      if (utf8_char[1] == 0x83 && utf8_char[2] == 0xbc) {
        /* U+30FC KATAKANA-HIRAGANA PROLONGED SOUND MARK */
        return GRN_TRUE;
      }
    } else if (utf8_char[0] == 0xef) {
      if (utf8_char[1] == 0xbd && utf8_char[2] == 0xb0) {
        /* U+FF70 HALFWIDTH KATAKANA-HIRAGANA PROLONGED SOUND MARK */
        return GRN_TRUE;
      }
    }
  }

  return GRN_FALSE;
}

grn_inline static grn_bool
utf8_normalize_is_middle_dot_family(const unsigned char *utf8_char,
                                    size_t length)
{
  if (length == 3) {
    if (utf8_char[0] == 0xe1) {
      if (utf8_char[1] == 0x90 && utf8_char[2] == 0xa7) {
        /* U+1427 CANADIAN SYLLABICS FINAL MIDDLE DOT */
        return GRN_TRUE;
      }
    } else if (utf8_char[0] == 0xe2) {
      if (utf8_char[1] == 0x80 && utf8_char[2] == 0xa2) {
        /* U+2022 BULLET */
        return GRN_TRUE;
      } else if (utf8_char[1] == 0x88 && utf8_char[2] == 0x99) {
        /* U+2219 BULLET OPERATOR */
        return GRN_TRUE;
      } else if (utf8_char[1] == 0x8b && utf8_char[2] == 0x85) {
        /* U+22C5 DOT OPERATOR */
        return GRN_TRUE;
      } else if (utf8_char[1] == 0xb8 && utf8_char[2] == 0xb1) {
        /* U+2E31 WORD SEPARATOR MIDDLE DOT */
        return GRN_TRUE;
      }
    } else if (utf8_char[0] == 0xe3) {
      if (utf8_char[1] == 0x83 && utf8_char[2] == 0xbb) {
        /* U+30FB KATAKANA MIDDLE DOT */
        return GRN_TRUE;
      }
    } else if (utf8_char[0] == 0xef) {
      if (utf8_char[1] == 0xbd && utf8_char[2] == 0xa5) {
        /* U+FF65 HALFWIDTH KATAKANA MIDDLE DOT */
        return GRN_TRUE;
      }
    }
  }

  return GRN_FALSE;
}

grn_inline static grn_bool
utf8_normalize_unify_katakana_v_sounds(const unsigned char *utf8_char,
                                       size_t length,
                                       unsigned char *previous_normalized,
                                       unsigned char *normalized)
{
  if (!previous_normalized) {
    return GRN_FALSE;
  }

  {
    size_t previous_length = normalized - previous_normalized;

    /* U+30F4 KATAKANA LETTER VU */
    if (previous_length == 3 &&
        previous_normalized[0] == 0xe3 &&
        previous_normalized[1] == 0x83 &&
        previous_normalized[2] == 0xb4) {
      if (length == 3 && utf8_char[0] == 0xe3 && utf8_char[1] == 0x82) {
        if (utf8_char[2] == 0xa1) {        /* U+30A1 KATAKANA LETTER SMALL A */
          /* U+30D0 KATAKANA LETTER BA */
          previous_normalized[2] = 0x90;
          return GRN_TRUE;
        } else if (utf8_char[2] == 0xa3) { /* U+30A3 KATAKANA LETTER SMALL I */
          /* U+30D3 KATAKANA LETTER BI */
          previous_normalized[2] = 0x93;
          return GRN_TRUE;
        } else if (utf8_char[2] == 0xa5) { /* U+30A5 KATAKANA LETTER SMALL U */
          /* U+30D6 KATAKANA LETTER BU */
          previous_normalized[2] = 0x96;
          return GRN_TRUE;
        } else if (utf8_char[2] == 0xa7) { /* U+30A7 KATAKANA LETTER SMALL E */
          /* U+30D9 KATAKANA LETTER BE */
          previous_normalized[2] = 0x99;
          return GRN_TRUE;
        } else if (utf8_char[2] == 0xa9) { /* U+30A8 KATAKANA LETTER SMALL O */
          /* U+30DC KATAKANA LETTER BO */
          previous_normalized[2] = 0x9c;
          return GRN_TRUE;
        }
      }
      /* U+30D6 KATAKANA LETTER BU */
      previous_normalized[2] = 0x96;
    }
  }

  return GRN_FALSE;
}

grn_inline static grn_bool
utf8_normalize_unify_katakana_bu_sound(const unsigned char *utf8_char,
                                       size_t length,
                                       unsigned char *previous_normalized,
                                       unsigned char *normalized)
{
  if (!previous_normalized) {
    return GRN_FALSE;
  }

  {
    size_t previous_length = normalized - previous_normalized;

    /* U+30F4 KATAKANA LETTER VU */
    if (previous_length == 3 &&
        previous_normalized[0] == 0xe3 &&
        previous_normalized[1] == 0x83 &&
        previous_normalized[2] == 0xb4) {
      /* U+30D6 KATAKANA LETTER BU */
      previous_normalized[2] = 0x96;
      if (length == 3 &&
          utf8_char[0] == 0xe3 &&
          utf8_char[1] == 0x82 &&
          /* U+30A1 KATAKANA LETTER SMALL A */
          /* U+30A3 KATAKANA LETTER SMALL I */
          /* U+30A5 KATAKANA LETTER SMALL U */
          /* U+30A7 KATAKANA LETTER SMALL E */
          /* U+30A9 KATAKANA LETTER SMALL O */
          (utf8_char[2] == 0xa1 ||
           utf8_char[2] == 0xa3 ||
           utf8_char[2] == 0xa5 ||
           utf8_char[2] == 0xa7 ||
           utf8_char[2] == 0xa9)) {
        return GRN_TRUE;
      }
    }
  }

  return GRN_FALSE;
}

grn_inline static grn_obj *
utf8_normalize(grn_ctx *ctx,
               grn_string *nstr,
               grn_utf8_normalize_options *options)
{
  int16_t *ch;
  const unsigned char *s, *s_, *s__ = NULL, *p, *p2, *pe, *e;
  unsigned char *d, *d_, *de;
  uint_least8_t *cp;
  uint64_t *offsets;
  size_t length = 0, ls, lp, size = nstr->original_length_in_bytes, ds = size * 3;
  int removeblankp = nstr->flags & GRN_STRING_REMOVE_BLANK;
  grn_bool remove_tokenized_delimiter_p =
    nstr->flags & GRN_STRING_REMOVE_TOKENIZED_DELIMITER;
  if (!(nstr->normalized = GRN_MALLOC(ds + 1))) {
    ERR(GRN_NO_MEMORY_AVAILABLE,
        "[string][utf8] failed to allocate normalized text space");
    return NULL;
  }
  if (nstr->flags & GRN_STRING_WITH_CHECKS) {
    if (!(nstr->checks = GRN_MALLOC(ds * sizeof(int16_t) + 1))) {
      GRN_FREE(nstr->normalized);
      nstr->normalized = NULL;
      ERR(GRN_NO_MEMORY_AVAILABLE,
          "[string][utf8] failed to allocate checks space");
      return NULL;
    }
  }
  ch = nstr->checks;
  if (nstr->flags & GRN_STRING_WITH_TYPES) {
    if (!(nstr->ctypes = GRN_MALLOC(ds + 1))) {
      if (nstr->checks) { GRN_FREE(nstr->checks); nstr->checks = NULL; }
      GRN_FREE(nstr->normalized); nstr->normalized = NULL;
      ERR(GRN_NO_MEMORY_AVAILABLE,
          "[string][utf8] failed to allocate character types space");
      return NULL;
    }
  }
  cp = nstr->ctypes;
  if (options->report_source_offset) {
    if (!(nstr->offsets = GRN_MALLOC(sizeof(uint64_t) * (ds + 1)))) {
      if (nstr->checks) { GRN_FREE(nstr->checks); nstr->checks = NULL; }
      if (nstr->ctypes) { GRN_FREE(nstr->ctypes); nstr->ctypes = NULL; }
      GRN_FREE(nstr->normalized); nstr->normalized = NULL;
      ERR(GRN_NO_MEMORY_AVAILABLE,
          "[string][utf8] failed to allocate offsets space");
      return NULL;
    }
  }
  offsets = nstr->offsets;
  d = (unsigned char *)nstr->normalized;
  de = d + ds;
  d_ = NULL;
  e = (unsigned char *)nstr->original + size;
  for (s = s_ = (unsigned char *)nstr->original; ; s += ls) {
    if (!(ls = grn_str_charlen_utf8(ctx, s, e))) {
      break;
    }
    if (remove_tokenized_delimiter_p &&
        grn_tokenizer_is_tokenized_delimiter(ctx, (const char *)s, ls,
                                             GRN_ENC_UTF8)) {
      continue;
    }
    if ((p = (unsigned char *)options->decompose_func(s))) {
      pe = p + strlen((char *)p);
    } else {
      p = s;
      pe = p + ls;
    }
    if (d_ && (p2 = (unsigned char *)options->compose_func(d_, p))) {
      p = p2;
      pe = p + strlen((char *)p);
      if (cp) { cp--; }
      if (ch) {
        ch -= (d - d_);
        if (ch[0] >= 0) {
          s_ = s__;
        }
      }
      if (offsets) {
        offsets--;
      }
      d = d_;
      length--;
    }
    for (; ; p += lp) {
      if (!(lp = grn_str_charlen_utf8(ctx, p, pe))) {
        break;
      }
      if ((*p == ' ' && removeblankp) || *p < 0x20  /* skip unprintable ascii */ ) {
        if (cp > nstr->ctypes) { *(cp - 1) |= GRN_CHAR_BLANK; }
        if (!options->include_removed_source_location) {
          s_ += lp;
        }
      } else {
        size_t lp_original = lp;
        grn_char_type char_type;
        char_type = options->char_type_func(p);

        if (de <= d + lp) {
          unsigned char *normalized;
          ds += (ds >> 1) + lp;
          if (!(normalized = GRN_REALLOC(nstr->normalized, ds + 1))) {
            if (nstr->ctypes) { GRN_FREE(nstr->ctypes); nstr->ctypes = NULL; }
            if (nstr->checks) { GRN_FREE(nstr->checks); nstr->checks = NULL; }
            if (nstr->offsets) { GRN_FREE(nstr->offsets); nstr->offsets = NULL; }
            GRN_FREE(nstr->normalized); nstr->normalized = NULL;
            ERR(GRN_NO_MEMORY_AVAILABLE,
                "[string][utf8] failed to expand normalized text space");
            return NULL;
          }
          de = normalized + ds;
          d = normalized + (d - (unsigned char *)nstr->normalized);
          nstr->normalized = (char *)normalized;
          if (ch) {
            int16_t *checks;
            if (!(checks = GRN_REALLOC(nstr->checks, ds * sizeof(int16_t) + 1))) {
              if (nstr->ctypes) { GRN_FREE(nstr->ctypes); nstr->ctypes = NULL; }
              if (nstr->offsets) { GRN_FREE(nstr->offsets); nstr->offsets = NULL; }
              GRN_FREE(nstr->checks); nstr->checks = NULL;
              GRN_FREE(nstr->normalized); nstr->normalized = NULL;
              ERR(GRN_NO_MEMORY_AVAILABLE,
                  "[string][utf8] failed to expand checks space");
              return NULL;
            }
            ch = checks + (ch - nstr->checks);
            nstr->checks = checks;
          }
          if (cp) {
            uint_least8_t *ctypes;
            if (!(ctypes = GRN_REALLOC(nstr->ctypes, ds + 1))) {
              GRN_FREE(nstr->ctypes); nstr->ctypes = NULL;
              if (nstr->checks) { GRN_FREE(nstr->checks); nstr->checks = NULL; }
              if (nstr->offsets) { GRN_FREE(nstr->offsets); nstr->offsets = NULL; }
              GRN_FREE(nstr->normalized); nstr->normalized = NULL;
              ERR(GRN_NO_MEMORY_AVAILABLE,
                  "[string][utf8] failed to expand character types space");
              return NULL;
            }
            cp = ctypes + (cp - nstr->ctypes);
            nstr->ctypes = ctypes;
          }
          if (offsets) {
            uint64_t *new_offsets;
            if (!(new_offsets = GRN_REALLOC(nstr->offsets,
                                            sizeof(uint64_t) * (ds + 1)))) {
              GRN_FREE(nstr->offsets); nstr->offsets = NULL;
              if (nstr->ctypes) { GRN_FREE(nstr->ctypes); nstr->ctypes = NULL; }
              if (nstr->checks) { GRN_FREE(nstr->checks); nstr->checks = NULL; }
              GRN_FREE(nstr->normalized); nstr->normalized = NULL;
              ERR(GRN_NO_MEMORY_AVAILABLE,
                  "[string][utf8] failed to expand offsets space");
              return NULL;
            }
            offsets = new_offsets + (offsets - nstr->offsets);
            nstr->offsets = new_offsets;
          }
        }

        {
          const unsigned char *p_original = p;
          unsigned char unified_kana[3];
          unsigned char unified_kana_case[3];
          unsigned char unified_kana_voiced_sound_mark[3];
          const unsigned char unified_hyphen[] = {'-'};
          /* U+30FC KATAKANA-HIRAGANA PROLONGED SOUND MARK */
          const unsigned char unified_prolonged_sound_mark[] =
            {0xe3, 0x83, 0xbc};
          /* U+00B7 MIDDLE DOT */
          const unsigned char unified_middle_dot[] = {0xc2, 0xb7};

          if (options->unify_kana &&
              char_type == GRN_CHAR_KATAKANA &&
              lp == 3) {
            p = utf8_normalize_unify_kana(p, unified_kana);
            if (p == unified_kana) {
              char_type = GRN_CHAR_HIRAGANA;
            }
          }

          if (options->unify_kana_case) {
            switch (char_type) {
            case GRN_CHAR_HIRAGANA :
              if (lp == 3) {
                p = utf8_normalize_unify_hiragana_case(p, unified_kana_case);
              }
              break;
            case GRN_CHAR_KATAKANA :
              if (lp == 3) {
                p = utf8_normalize_unify_katakana_case(p, unified_kana_case);
              }
              break;
            default :
              break;
            }
          }

          if (options->unify_kana_voiced_sound_mark) {
            switch (char_type) {
            case GRN_CHAR_HIRAGANA :
              if (lp == 3) {
                p = utf8_normalize_unify_hiragana_voiced_sound_mark(
                  p, unified_kana_voiced_sound_mark);
              }
              break;
            case GRN_CHAR_KATAKANA :
              if (lp == 3) {
                p = utf8_normalize_unify_katakana_voiced_sound_mark(
                  p, unified_kana_voiced_sound_mark);
              }
              break;
            default :
              break;
            }
          }

          if (options->unify_hyphen) {
            if (utf8_normalize_is_hyphen_famity(p, lp)) {
              p = unified_hyphen;
              lp = sizeof(unified_hyphen);
              char_type = GRN_CHAR_SYMBOL;
            }
          }

          if (options->unify_prolonged_sound_mark) {
            if (utf8_normalize_is_prolonged_sound_mark_famity(p, lp)) {
              p = unified_prolonged_sound_mark;
              lp = sizeof(unified_prolonged_sound_mark);
              char_type = GRN_CHAR_KATAKANA;
            }
          }

          if (options->unify_hyphen_and_prolonged_sound_mark) {
            if (utf8_normalize_is_hyphen_famity(p, lp) ||
                utf8_normalize_is_prolonged_sound_mark_famity(p, lp)) {
              p = unified_hyphen;
              lp = sizeof(unified_hyphen);
              char_type = GRN_CHAR_SYMBOL;
            }
          }

          if (options->unify_middle_dot) {
            if (utf8_normalize_is_middle_dot_family(p, lp)) {
              p = unified_middle_dot;
              lp = sizeof(unified_middle_dot);
              char_type = GRN_CHAR_SYMBOL;
            }
          }

          if (options->unify_katakana_v_sounds) {
            if (utf8_normalize_unify_katakana_v_sounds(p, lp, d_, d)) {
              lp = 0;
            }
          }

          if (options->unify_katakana_bu_sound) {
            if (utf8_normalize_unify_katakana_bu_sound(p, lp, d_, d)) {
              lp = 0;
            }
          }

          grn_memcpy(d, p, lp);
          p = p_original;
        }
        d_ = d;
        if (lp > 0) {
          d += lp;
          length++;
          if (cp) { *cp++ = char_type; }
          if (ch) {
            size_t i;
            if (s_ == s + ls) {
              *ch++ = -1;
            } else {
              *ch++ = (int16_t)(s + ls - s_);
              s__ = s_;
              s_ = s + ls;
            }
            for (i = lp; i > 1; i--) { *ch++ = 0; }
          }
          if (offsets) {
            *offsets++ = (uint64_t)(s - (const unsigned char *)nstr->original);
          }
        }
        lp = lp_original;
      }
    }
  }
  if (cp) { *cp = GRN_CHAR_NULL; }
  if (offsets) { *offsets = nstr->original_length_in_bytes; }
  if (options->unify_katakana_v_sounds) {
    utf8_normalize_unify_katakana_v_sounds(NULL, 0, d_, d);
  }
  if (options->unify_katakana_bu_sound) {
    utf8_normalize_unify_katakana_bu_sound(NULL, 0, d_, d);
  }
  *d = '\0';
  nstr->n_characters = length;
  nstr->normalized_length_in_bytes = (size_t)(d - (unsigned char *)nstr->normalized);
  return NULL;
}
#endif /* GRN_WITH_NFKC */

grn_inline static grn_obj *
ascii_normalize(grn_ctx *ctx, grn_string *nstr)
{
  int16_t *ch;
  const unsigned char *s, *s_, *e;
  unsigned char *d, *d0, *d_;
  uint_least8_t *cp, *ctypes, ctype;
  size_t size = nstr->original_length_in_bytes, length = 0;
  int removeblankp = nstr->flags & GRN_STRING_REMOVE_BLANK;
  if (!(nstr->normalized = GRN_MALLOC(size + 1))) {
    ERR(GRN_NO_MEMORY_AVAILABLE,
        "[string][ascii] failed to allocate normalized text space");
    return NULL;
  }
  d0 = (unsigned char *) nstr->normalized;
  if (nstr->flags & GRN_STRING_WITH_CHECKS) {
    if (!(nstr->checks = GRN_MALLOC(size * sizeof(int16_t) + 1))) {
      GRN_FREE(nstr->normalized);
      nstr->normalized = NULL;
      ERR(GRN_NO_MEMORY_AVAILABLE,
          "[string][ascii] failed to allocate checks space");
      return NULL;
    }
  }
  ch = nstr->checks;
  if (nstr->flags & GRN_STRING_WITH_TYPES) {
    if (!(nstr->ctypes = GRN_MALLOC(size + 1))) {
      GRN_FREE(nstr->checks);
      GRN_FREE(nstr->normalized);
      nstr->checks = NULL;
      nstr->normalized = NULL;
      ERR(GRN_NO_MEMORY_AVAILABLE,
          "[string][ascii] failed to allocate character types space");
      return NULL;
    }
  }
  cp = ctypes = nstr->ctypes;
  e = (unsigned char *)nstr->original + size;
  for (s = s_ = (unsigned char *) nstr->original, d = d_ = d0; s < e; s++) {
    unsigned char c = *s;
    switch (c >> 4) {
    case 0 :
    case 1 :
      /* skip unprintable ascii */
      if (cp > ctypes) { *(cp - 1) |= GRN_CHAR_BLANK; }
      continue;
    case 2 :
      if (c == 0x20) {
        if (removeblankp) {
          if (cp > ctypes) { *(cp - 1) |= GRN_CHAR_BLANK; }
          continue;
        } else {
          *d = ' ';
          ctype = GRN_CHAR_BLANK|GRN_CHAR_SYMBOL;
        }
      } else {
        *d = c;
        ctype = GRN_CHAR_SYMBOL;
      }
      break;
    case 3 :
      *d = c;
      ctype = (c <= 0x39) ? GRN_CHAR_DIGIT : GRN_CHAR_SYMBOL;
      break;
    case 4 :
      *d = ('A' <= c) ? c + 0x20 : c;
      ctype = (c == 0x40) ? GRN_CHAR_SYMBOL : GRN_CHAR_ALPHA;
      break;
    case 5 :
      *d = (c <= 'Z') ? c + 0x20 : c;
      ctype = (c <= 0x5a) ? GRN_CHAR_ALPHA : GRN_CHAR_SYMBOL;
      break;
    case 6 :
      *d = c;
      ctype = (c == 0x60) ? GRN_CHAR_SYMBOL : GRN_CHAR_ALPHA;
      break;
    case 7 :
      *d = c;
      ctype = (c <= 0x7a) ? GRN_CHAR_ALPHA : (c == 0x7f ? GRN_CHAR_OTHERS : GRN_CHAR_SYMBOL);
      break;
    default :
      *d = c;
      ctype = GRN_CHAR_OTHERS;
      break;
    }
    d++;
    length++;
    if (cp) { *cp++ = ctype; }
    if (ch) {
      *ch++ = (int16_t)(s + 1 - s_);
      s_ = s + 1;
      while (++d_ < d) { *ch++ = 0; }
    }
  }
  if (cp) { *cp = GRN_CHAR_NULL; }
  *d = '\0';
  nstr->n_characters = length;
  nstr->normalized_length_in_bytes = (size_t)(d - (unsigned char *)nstr->normalized);
  return NULL;
}

/* use cp1252 as latin1 */
grn_inline static grn_obj *
latin1_normalize(grn_ctx *ctx, grn_string *nstr)
{
  int16_t *ch;
  const unsigned char *s, *s_, *e;
  unsigned char *d, *d0, *d_;
  uint_least8_t *cp, *ctypes, ctype;
  size_t size = nstr->original_length_in_bytes, length = 0;
  int removeblankp = nstr->flags & GRN_STRING_REMOVE_BLANK;
  if (!(nstr->normalized = GRN_MALLOC(size + 1))) {
    ERR(GRN_NO_MEMORY_AVAILABLE,
        "[string][latin1] failed to allocate normalized text space");
    return NULL;
  }
  d0 = (unsigned char *) nstr->normalized;
  if (nstr->flags & GRN_STRING_WITH_CHECKS) {
    if (!(nstr->checks = GRN_MALLOC(size * sizeof(int16_t) + 1))) {
      GRN_FREE(nstr->normalized);
      nstr->normalized = NULL;
      ERR(GRN_NO_MEMORY_AVAILABLE,
          "[string][latin1] failed to allocate checks space");
      return NULL;
    }
  }
  ch = nstr->checks;
  if (nstr->flags & GRN_STRING_WITH_TYPES) {
    if (!(nstr->ctypes = GRN_MALLOC(size + 1))) {
      GRN_FREE(nstr->checks);
      GRN_FREE(nstr->normalized);
      nstr->checks = NULL;
      nstr->normalized = NULL;
      ERR(GRN_NO_MEMORY_AVAILABLE,
          "[normalizer][latin1] failed to allocate character types space");
      return NULL;
    }
  }
  cp = ctypes = nstr->ctypes;
  e = (unsigned char *)nstr->original + size;
  for (s = s_ = (unsigned char *) nstr->original, d = d_ = d0; s < e; s++) {
    unsigned char c = *s;
    switch (c >> 4) {
    case 0 :
    case 1 :
      /* skip unprintable ascii */
      if (cp > ctypes) { *(cp - 1) |= GRN_CHAR_BLANK; }
      continue;
    case 2 :
      if (c == 0x20) {
        if (removeblankp) {
          if (cp > ctypes) { *(cp - 1) |= GRN_CHAR_BLANK; }
          continue;
        } else {
          *d = ' ';
          ctype = GRN_CHAR_BLANK|GRN_CHAR_SYMBOL;
        }
      } else {
        *d = c;
        ctype = GRN_CHAR_SYMBOL;
      }
      break;
    case 3 :
      *d = c;
      ctype = (c <= 0x39) ? GRN_CHAR_DIGIT : GRN_CHAR_SYMBOL;
      break;
    case 4 :
      *d = ('A' <= c) ? c + 0x20 : c;
      ctype = (c == 0x40) ? GRN_CHAR_SYMBOL : GRN_CHAR_ALPHA;
      break;
    case 5 :
      *d = (c <= 'Z') ? c + 0x20 : c;
      ctype = (c <= 0x5a) ? GRN_CHAR_ALPHA : GRN_CHAR_SYMBOL;
      break;
    case 6 :
      *d = c;
      ctype = (c == 0x60) ? GRN_CHAR_SYMBOL : GRN_CHAR_ALPHA;
      break;
    case 7 :
      *d = c;
      ctype = (c <= 0x7a) ? GRN_CHAR_ALPHA : (c == 0x7f ? GRN_CHAR_OTHERS : GRN_CHAR_SYMBOL);
      break;
    case 8 :
      if (c == 0x8a || c == 0x8c || c == 0x8e) {
        *d = c + 0x10;
        ctype = GRN_CHAR_ALPHA;
      } else {
        *d = c;
        ctype = GRN_CHAR_SYMBOL;
      }
      break;
    case 9 :
      if (c == 0x9a || c == 0x9c || c == 0x9e || c == 0x9f) {
        *d = (c == 0x9f) ? c + 0x60 : c;
        ctype = GRN_CHAR_ALPHA;
      } else {
        *d = c;
        ctype = GRN_CHAR_SYMBOL;
      }
      break;
    case 0x0c :
      *d = c + 0x20;
      ctype = GRN_CHAR_ALPHA;
      break;
    case 0x0d :
      *d = (c == 0xd7 || c == 0xdf) ? c : c + 0x20;
      ctype = (c == 0xd7) ? GRN_CHAR_SYMBOL : GRN_CHAR_ALPHA;
      break;
    case 0x0e :
      *d = c;
      ctype = GRN_CHAR_ALPHA;
      break;
    case 0x0f :
      *d = c;
      ctype = (c == 0xf7) ? GRN_CHAR_SYMBOL : GRN_CHAR_ALPHA;
      break;
    default :
      *d = c;
      ctype = GRN_CHAR_OTHERS;
      break;
    }
    d++;
    length++;
    if (cp) { *cp++ = ctype; }
    if (ch) {
      *ch++ = (int16_t)(s + 1 - s_);
      s_ = s + 1;
      while (++d_ < d) { *ch++ = 0; }
    }
  }
  if (cp) { *cp = GRN_CHAR_NULL; }
  *d = '\0';
  nstr->n_characters = length;
  nstr->normalized_length_in_bytes = (size_t)(d - (unsigned char *)nstr->normalized);
  return NULL;
}

grn_inline static grn_obj *
koi8r_normalize(grn_ctx *ctx, grn_string *nstr)
{
  int16_t *ch;
  const unsigned char *s, *s_, *e;
  unsigned char *d, *d0, *d_;
  uint_least8_t *cp, *ctypes, ctype;
  size_t size = nstr->original_length_in_bytes, length = 0;
  int removeblankp = nstr->flags & GRN_STRING_REMOVE_BLANK;
  if (!(nstr->normalized = GRN_MALLOC(size + 1))) {
    ERR(GRN_NO_MEMORY_AVAILABLE,
        "[string][koi8r] failed to allocate normalized text space");
    return NULL;
  }
  d0 = (unsigned char *) nstr->normalized;
  if (nstr->flags & GRN_STRING_WITH_CHECKS) {
    if (!(nstr->checks = GRN_MALLOC(size * sizeof(int16_t) + 1))) {
      GRN_FREE(nstr->normalized);
      nstr->normalized = NULL;
      ERR(GRN_NO_MEMORY_AVAILABLE,
          "[string][koi8r] failed to allocate checks space");
      return NULL;
    }
  }
  ch = nstr->checks;
  if (nstr->flags & GRN_STRING_WITH_TYPES) {
    if (!(nstr->ctypes = GRN_MALLOC(size + 1))) {
      GRN_FREE(nstr->checks);
      GRN_FREE(nstr->normalized);
      nstr->checks = NULL;
      nstr->normalized = NULL;
      ERR(GRN_NO_MEMORY_AVAILABLE,
          "[string][koi8r] failed to allocate character types space");
      return NULL;
    }
  }
  cp = ctypes = nstr->ctypes;
  e = (unsigned char *)nstr->original + size;
  for (s = s_ = (unsigned char *) nstr->original, d = d_ = d0; s < e; s++) {
    unsigned char c = *s;
    switch (c >> 4) {
    case 0 :
    case 1 :
      /* skip unprintable ascii */
      if (cp > ctypes) { *(cp - 1) |= GRN_CHAR_BLANK; }
      continue;
    case 2 :
      if (c == 0x20) {
        if (removeblankp) {
          if (cp > ctypes) { *(cp - 1) |= GRN_CHAR_BLANK; }
          continue;
        } else {
          *d = ' ';
          ctype = GRN_CHAR_BLANK|GRN_CHAR_SYMBOL;
        }
      } else {
        *d = c;
        ctype = GRN_CHAR_SYMBOL;
      }
      break;
    case 3 :
      *d = c;
      ctype = (c <= 0x39) ? GRN_CHAR_DIGIT : GRN_CHAR_SYMBOL;
      break;
    case 4 :
      *d = ('A' <= c) ? c + 0x20 : c;
      ctype = (c == 0x40) ? GRN_CHAR_SYMBOL : GRN_CHAR_ALPHA;
      break;
    case 5 :
      *d = (c <= 'Z') ? c + 0x20 : c;
      ctype = (c <= 0x5a) ? GRN_CHAR_ALPHA : GRN_CHAR_SYMBOL;
      break;
    case 6 :
      *d = c;
      ctype = (c == 0x60) ? GRN_CHAR_SYMBOL : GRN_CHAR_ALPHA;
      break;
    case 7 :
      *d = c;
      ctype = (c <= 0x7a) ? GRN_CHAR_ALPHA : (c == 0x7f ? GRN_CHAR_OTHERS : GRN_CHAR_SYMBOL);
      break;
    case 0x0a :
      *d = c;
      ctype = (c == 0xa3) ? GRN_CHAR_ALPHA : GRN_CHAR_OTHERS;
      break;
    case 0x0b :
      if (c == 0xb3) {
        *d = c - 0x10;
        ctype = GRN_CHAR_ALPHA;
      } else {
        *d = c;
        ctype = GRN_CHAR_OTHERS;
      }
      break;
    case 0x0c :
    case 0x0d :
      *d = c;
      ctype = GRN_CHAR_ALPHA;
      break;
    case 0x0e :
    case 0x0f :
      *d = c - 0x20;
      ctype = GRN_CHAR_ALPHA;
      break;
    default :
      *d = c;
      ctype = GRN_CHAR_OTHERS;
      break;
    }
    d++;
    length++;
    if (cp) { *cp++ = ctype; }
    if (ch) {
      *ch++ = (int16_t)(s + 1 - s_);
      s_ = s + 1;
      while (++d_ < d) { *ch++ = 0; }
    }
  }
  if (cp) { *cp = GRN_CHAR_NULL; }
  *d = '\0';
  nstr->n_characters = length;
  nstr->normalized_length_in_bytes = (size_t)(d - (unsigned char *)nstr->normalized);
  return NULL;
}

static grn_obj *
auto_next(grn_ctx *ctx, int nargs, grn_obj **args, grn_user_data *user_data)
{
  grn_string *string = (grn_string *)(args[0]);
  switch (string->encoding) {
  case GRN_ENC_EUC_JP :
    eucjp_normalize(ctx, string);
    break;
  case GRN_ENC_UTF8 :
#ifdef GRN_WITH_NFKC
    {
      grn_utf8_normalize_options options;
      utf8_normalize_options_init(&options,
                                  grn_nfkc_char_type,
                                  grn_nfkc_decompose,
                                  grn_nfkc_compose);
      utf8_normalize(ctx, string, &options);
    }
#else /* GRN_WITH_NFKC */
    ascii_normalize(ctx, string);
#endif /* GRN_WITH_NFKC */
    break;
  case GRN_ENC_SJIS :
    sjis_normalize(ctx, string);
    break;
  case GRN_ENC_LATIN1 :
    latin1_normalize(ctx, string);
    break;
  case GRN_ENC_KOI8R :
    koi8r_normalize(ctx, string);
    break;
  default :
    ascii_normalize(ctx, string);
    break;
  }
  return NULL;
}

#ifdef GRN_WITH_NFKC
static grn_obj *
nfkc51_next(grn_ctx *ctx, int nargs, grn_obj **args, grn_user_data *user_data)
{
  grn_string *string = (grn_string *)(args[0]);
  grn_utf8_normalize_options options;

  utf8_normalize_options_init(&options,
                              grn_nfkc50_char_type,
                              grn_nfkc50_decompose,
                              grn_nfkc50_compose);
  utf8_normalize(ctx, string, &options);
  return NULL;
}

static void *
nfkc100_open_options(grn_ctx *ctx,
                     grn_obj *string,
                     grn_obj *raw_options,
                     void *user_data)
{
  grn_utf8_normalize_options *options;

  options = GRN_MALLOC(sizeof(grn_utf8_normalize_options));
  if (!options) {
    ERR(GRN_NO_MEMORY_AVAILABLE,
        "[normalizer][nfkc100] "
        "failed to allocate memory for options");
    return NULL;
  }

  utf8_normalize_options_init(options,
                              grn_nfkc100_char_type,
                              grn_nfkc100_decompose,
                              grn_nfkc100_compose);

  GRN_OPTION_VALUES_EACH_BEGIN(ctx, raw_options, i, name, name_length) {
    grn_raw_string name_raw;
    name_raw.value = name;
    name_raw.length = name_length;

    if (GRN_RAW_STRING_EQUAL_CSTRING(name_raw,
                                     "include_removed_source_location")) {
      options->include_removed_source_location =
        grn_vector_get_element_bool(ctx,
                                    raw_options,
                                    i,
                                    options->include_removed_source_location);
    } else if (GRN_RAW_STRING_EQUAL_CSTRING(name_raw, "report_source_offset")) {
      options->report_source_offset =
        grn_vector_get_element_bool(ctx,
                                    raw_options,
                                    i,
                                    options->report_source_offset);
    } else if (GRN_RAW_STRING_EQUAL_CSTRING(name_raw, "unify_kana")) {
      options->unify_kana = grn_vector_get_element_bool(ctx,
                                                        raw_options,
                                                        i,
                                                        options->unify_kana);
    } else if (GRN_RAW_STRING_EQUAL_CSTRING(name_raw, "unify_kana_case")) {
      options->unify_kana_case =
        grn_vector_get_element_bool(ctx,
                                    raw_options,
                                    i,
                                    options->unify_kana_case);
    } else if (GRN_RAW_STRING_EQUAL_CSTRING(name_raw,
                                            "unify_kana_voiced_sound_mark")) {
      options->unify_kana_voiced_sound_mark =
        grn_vector_get_element_bool(ctx,
                                    raw_options,
                                    i,
                                    options->unify_kana_voiced_sound_mark);
    } else if (GRN_RAW_STRING_EQUAL_CSTRING(name_raw, "unify_hyphen")) {
      options->unify_hyphen = grn_vector_get_element_bool(ctx,
                                                          raw_options,
                                                          i,
                                                          options->unify_hyphen);
    } else if (GRN_RAW_STRING_EQUAL_CSTRING(name_raw,
                                            "unify_prolonged_sound_mark")) {
      options->unify_prolonged_sound_mark =
        grn_vector_get_element_bool(ctx,
                                    raw_options,
                                    i,
                                    options->unify_prolonged_sound_mark);
    } else if (GRN_RAW_STRING_EQUAL_CSTRING(name_raw,
                                            "unify_hyphen_and_prolonged_sound_mark")) {
      options->unify_hyphen_and_prolonged_sound_mark =
        grn_vector_get_element_bool(ctx,
                                    raw_options,
                                    i,
                                    options->unify_hyphen_and_prolonged_sound_mark);
    } else if (GRN_RAW_STRING_EQUAL_CSTRING(name_raw, "unify_middle_dot")) {
      options->unify_middle_dot =
        grn_vector_get_element_bool(ctx,
                                    raw_options,
                                    i,
                                    options->unify_middle_dot);
    } else if (GRN_RAW_STRING_EQUAL_CSTRING(name_raw, "unify_katakana_v_sounds")) {
      options->unify_katakana_v_sounds =
        grn_vector_get_element_bool(ctx,
                                    raw_options,
                                    i,
                                    options->unify_katakana_v_sounds);
    } else if (GRN_RAW_STRING_EQUAL_CSTRING(name_raw, "unify_katakana_bu_sound")) {
      options->unify_katakana_bu_sound =
        grn_vector_get_element_bool(ctx,
                                    raw_options,
                                    i,
                                    options->unify_katakana_bu_sound);
    }
  } GRN_OPTION_VALUES_EACH_END();

  return options;
}

static void
nfkc100_close_options(grn_ctx *ctx, void *data)
{
  grn_utf8_normalize_options *options = data;
  GRN_FREE(options);
}

static grn_obj *
nfkc100_next(grn_ctx *ctx, int nargs, grn_obj **args, grn_user_data *user_data)
{
  grn_obj *string = args[0];
  grn_string *string_ = (grn_string *)string;
  grn_obj *table;
  grn_utf8_normalize_options *options;
  grn_utf8_normalize_options options_raw;

  table = grn_string_get_table(ctx, string);
  if (table) {
    options = grn_table_cache_normalizer_options(ctx,
                                                 table,
                                                 string,
                                                 nfkc100_open_options,
                                                 nfkc100_close_options,
                                                 NULL);
    if (ctx->rc != GRN_SUCCESS) {
      return NULL;
    }
  } else {
    utf8_normalize_options_init(&options_raw,
                                grn_nfkc100_char_type,
                                grn_nfkc100_decompose,
                                grn_nfkc100_compose);
    options = &options_raw;
  }

  utf8_normalize(ctx, string_, options);
  return NULL;
}
#endif /* GRN_WITH_NFKC */

grn_rc
grn_normalizer_normalize(grn_ctx *ctx, grn_obj *normalizer, grn_obj *string)
{
  grn_rc rc;
  int nargs = 0;

  grn_ctx_push(ctx, string);
  nargs++;
  rc = grn_proc_call(ctx, normalizer, nargs, NULL);
  grn_ctx_pop(ctx);

  return rc;
}

grn_rc
grn_db_init_builtin_normalizers(grn_ctx *ctx)
{
  const char *normalizer_nfkc51_name = "NormalizerNFKC51";
  const char *normalizer_nfkc100_name = "NormalizerNFKC100";

  grn_normalizer_register(ctx, GRN_NORMALIZER_AUTO_NAME, -1,
                          NULL, auto_next, NULL);

#ifdef GRN_WITH_NFKC
  grn_normalizer_register(ctx, normalizer_nfkc51_name, -1,
                          NULL, nfkc51_next, NULL);
  grn_normalizer_register(ctx, normalizer_nfkc100_name, -1,
                          NULL, nfkc100_next, NULL);
#else /* GRN_WITH_NFKC */
  grn_normalizer_register(ctx, normalizer_nfkc51_name, -1,
                          NULL, NULL, NULL);
#endif /* GRN_WITH_NFKC */
/*
  grn_normalizer_register(ctx, "NormalizerUCA", -1,
                          NULL, uca_next, NULL);
*/

  return GRN_SUCCESS;
}
