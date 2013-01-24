#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <mruby.h>
#include <mruby/array.h>
#include <mruby/numeric.h>
#include <mruby/string.h>

#define ROUND_FLOAT(f_) ((f_) >= 0 ? (f_ + 0.5) : (f_ - 0.5))

#define CAST_FROM_STRING(str_p_, t_) *((t_*) (str_p_))

#ifdef MRB_ENDIAN_BIG
static const int native_endian = 0;
#else
static const int native_endian = 1;
#endif

enum pack_type {
  PACK_INTEGER = 0,
  PACK_FLOAT
};

#define ALL_ARGUMENT_SIZE -1

struct parse_options {
  /* length of array to pack/unpack */
  int pack_len;

  /* pack/unpack type */
  enum pack_type type;

  /* Fixnum size */
  int size;

  /* 1 for signed, 0 for unsigned */
  int sign;

  /* 1 for double, 0 for float */
  int is_double;

  /* 1 for little endian, 0 for big endian */
  int endian;
};

static void
parse_option(mrb_state* mrb, const char* tstr, int tstr_len, int* tstr_i,
             struct parse_options* opts)
{
  char c = tstr[(*tstr_i)++];
  int may_have_suffix = 0;

  switch (c) {
    case 'C':
      opts->pack_len = 1;
      opts->type = PACK_INTEGER;
      opts->sign = 0;
      opts->size = 1;
      opts->endian = native_endian;
      break;
    case 'c':
      opts->pack_len = 1;
      opts->type = PACK_INTEGER;
      opts->sign = 1;
      opts->size = 1;
      opts->endian = native_endian;
      break;
    case 'S':
      opts->pack_len = 1;
      opts->type = PACK_INTEGER;
      opts->sign = 0;
      opts->size = 2;
      opts->endian = native_endian;
      may_have_suffix = 1;
      break;
    case 's':
      opts->pack_len = 1;
      opts->type = PACK_INTEGER;
      opts->sign = 1;
      opts->size = 2;
      opts->endian = native_endian;
      may_have_suffix = 1;
      break;
    case 'L':
      opts->pack_len = 1;
      opts->type = PACK_INTEGER;
      opts->sign = 0;
      opts->size = 4;
      opts->endian = native_endian;
      may_have_suffix = 1;
      break;
    case 'l':
      opts->pack_len = 1;
      opts->type = PACK_INTEGER;
      opts->sign = 1;
      opts->size = 4;
      opts->endian = native_endian;
      may_have_suffix = 1;
      break;
    case 'I':
      opts->pack_len = 1;
      opts->type = PACK_INTEGER;
      opts->sign = 0;
      opts->size = sizeof(unsigned int);
      opts->endian = native_endian;
      may_have_suffix = 1;
      break;
    case 'i':
      opts->pack_len = 1;
      opts->type = PACK_INTEGER;
      opts->sign = 0;
      opts->size = sizeof(signed int);
      opts->endian = native_endian;
      may_have_suffix = 1;
      break;
    case 'Q':
      opts->pack_len = 1;
      opts->type = PACK_INTEGER;
      opts->sign = 0;
      opts->size = 8;
      opts->endian = native_endian;
      may_have_suffix = 1;
      break;
    case 'q':
      opts->pack_len = 1;
      opts->type = PACK_INTEGER;
      opts->sign = 1;
      opts->size = 8;
      opts->endian = native_endian;
      may_have_suffix = 1;
      break;
    case 'D':
    case 'd':
      opts->pack_len = 1;
      opts->type = PACK_FLOAT;
      opts->is_double = 1;
      /* Used by '*' */
      opts->size = sizeof(double);
      opts->endian = native_endian;
      break;
    case 'F':
    case 'f':
      opts->pack_len = 1;
      opts->type = PACK_FLOAT;
      opts->is_double = 0;
      opts->size = sizeof(float);
      opts->endian = native_endian;
      break;
    case 'E':
      opts->pack_len = 1;
      opts->type = PACK_FLOAT;
      opts->is_double = 1;
      opts->size = sizeof(double);
      opts->endian = 1;
      break;
    case 'e':
      opts->pack_len = 1;
      opts->type = PACK_FLOAT;
      opts->is_double = 0;
      opts->size = sizeof(float);
      opts->endian = 1;
      break;
    case 'G':
      opts->pack_len = 1;
      opts->type = PACK_FLOAT;
      opts->is_double = 1;
      opts->size = sizeof(double);
      opts->endian = 0;
      break;
    case 'g':
      opts->pack_len = 1;
      opts->type = PACK_FLOAT;
      opts->is_double = 0;
      opts->size = sizeof(float);
      opts->endian = 0;
      break;
    case '*':
      /* Uses type, sign, size from last run */
      if (*tstr_i == 1) {
        mrb_raise(mrb, E_ARGUMENT_ERROR,
                  "'*' must follow existing directives!");
      }
      opts->pack_len = ALL_ARGUMENT_SIZE;
      break;
  }

  if ((may_have_suffix == 1) && (opts->type == PACK_INTEGER)) {
    if (*tstr_i < tstr_len) {
      char next_c = tstr[*tstr_i];
      if ((next_c == '!') && ((*tstr_i + 1) < tstr_len)) {
        char next_next_c = tstr[*tstr + 1];
        if ((next_next_c == '<') || (next_next_c == '>')) {
          /* omit '!' if it is of the format "!<" or "!>" */
          next_c = next_next_c;
          (*tstr_i)++;
        }
      }

      if (next_c == '>') {
        (*tstr_i)++;
        /* big endian */
        opts->endian = 0;
      } else if (next_c == '<') {
        (*tstr_i)++;
        /* little endian */
        opts->endian = 1;
      } else if ((next_c == '_') || (next_c == '!')) {
        (*tstr_i)++;
        /* TODO: maybe we do not need this */
        switch (c) {
          case 'S':
            opts->size = sizeof(unsigned short);
            break;
          case 's':
            opts->size = sizeof(signed short);
            break;
          case 'L':
            opts->size = sizeof(unsigned long);
            break;
          case 'l':
            opts->size = sizeof(signed long);
            break;
        }
      }
    }
  }
}

/* Check given endian with native endian, swap the buffer if different */
static char*
check_endian(char* buf, int len, int endian)
{
  if (endian != native_endian) {
    int i;
    char ch;

    for (i = 0; i < len / 2; i++) {
      ch = buf[i];
      buf[i] = buf[len - i - 1];
      buf[len - i - 1] = ch;
    }
  }
  return buf;
}

static int32_t
convert_to_int32(mrb_state* mrb, mrb_value v, int sign)
{
  int32_t ret = 0;

  switch(mrb_type(v)) {
    case MRB_TT_FIXNUM:
      ret = (int32_t) mrb_fixnum(v);
      break;
    case MRB_TT_FLOAT:
      /*
       * NOTE: by default mruby use int32_t as the type of mrb_int.
       * As a result, any value greater than 2147483647 will be of
       * float type, if we are converting to an unsigned value.
       * Here we will casting floats back to integers to fix
       * this problem.
       * In addition, float=>int32 and float =>uint32 follow different
       * rules, we need to take care of each case separately.
       */
      if (sign == 1) {
        ret = (int32_t) ROUND_FLOAT(mrb_float(v));
      } else {
        ret = (int32_t) ((uint32_t) ROUND_FLOAT(mrb_float(v)));
      }
      break;
    default:
      {
        mrb_value tmp = mrb_convert_type(mrb, v, MRB_TT_FIXNUM,
                                         "Integer", "to_int");
        ret = (int32_t) mrb_fixnum(tmp);
      }
  }
  return ret;
}

/*
 * For some systems, including the JavaScript environment used
 * in Webruby, 64-bit operations are emulated and much slower
 * than 32-bit operations. So we would only convert fixnum into
 * 64-bit values when necessary.
 */
static int64_t
convert_to_int64(mrb_state* mrb, mrb_value v, int sign)
{
  int64_t ret = 0;

  switch(mrb_type(v)) {
    case MRB_TT_FIXNUM:
      ret = (int64_t) mrb_fixnum(v);
      break;
    case MRB_TT_FLOAT:
      if (sign == 1) {
        ret = (int64_t) ROUND_FLOAT(mrb_float(v));
      } else {
        ret = (int64_t) ((uint64_t) ROUND_FLOAT(mrb_float(v)));
      }
      break;
    default:
      {
        mrb_value tmp = mrb_convert_type(mrb, v, MRB_TT_FIXNUM,
                                         "Integer", "to_int");
        ret = (int64_t) mrb_fixnum(tmp);
      }
  }
  return ret;
}

static mrb_value
convert_from_int32(int32_t v)
{
  if (FIXABLE(v)) {
    return mrb_fixnum_value((mrb_int) v);
  } else {
    return mrb_float_value((mrb_float) v);
  }
}

static mrb_value
convert_from_int64(int64_t v)
{
  if (FIXABLE(v)) {
    return mrb_fixnum_value((mrb_int) v);
  } else {
    return mrb_float_value((mrb_float) v);
  }
}

static int
pack_fixnum(mrb_state* mrb, mrb_value v, int size, int sign, int endian,
            mrb_value ret_str)
{
  char buf[8];

  switch (size) {
    case 1:
      if (sign == 1) {
        *((char*) buf) = (char) convert_to_int32(mrb, v, sign);
      } else {
        *((unsigned char*) buf) = (unsigned char) convert_to_int32(mrb, v, sign);
      }
      mrb_str_cat(mrb, ret_str, check_endian(buf, 1, endian), 1);
      return 1;
    case 2:
      if (sign == 1) {
        *((int16_t*) buf) = (int16_t) convert_to_int32(mrb, v, sign);
      } else {
        *((uint16_t*) buf) = (uint16_t) convert_to_int32(mrb, v, sign);
      }
      mrb_str_cat(mrb, ret_str, check_endian(buf, 2, endian), 2);
      return 2;
    case 4:
      if (sign == 1) {
        *((int32_t*) buf) = (int32_t) convert_to_int32(mrb, v, sign);
      } else {
        *((uint32_t*) buf) = (uint32_t) convert_to_int32(mrb, v, sign);
      }
      mrb_str_cat(mrb, ret_str, check_endian(buf, 4, endian), 4);
      return 4;
    case 8:
      if (sign == 1) {
        *((int64_t*) buf) = (int64_t) convert_to_int64(mrb, v, sign);
      } else {
        *((uint64_t*) buf) = (uint64_t) convert_to_int64(mrb, v, sign);
      }
      mrb_str_cat(mrb, ret_str, check_endian(buf, 8, endian), 8);
      return 8;
    default:
      mrb_raisef(mrb, E_ARGUMENT_ERROR, "Cannot pack a fixnum with size %d!",
                 size);
  }

  /* Actually this is not reachable. */
  return 0;
}

static mrb_value
unpack_fixnum(mrb_state* mrb, int size, int sign, int endian,
              char* str, int* str_i)
{
  mrb_value ret;
  char buf[8];

  switch (size) {
    case 1:
      memcpy(buf, str + (*str_i), 1);
      *str_i += 1;
      check_endian(buf, 1, endian);

      if (sign == 1) {
        ret = convert_from_int32(CAST_FROM_STRING(buf, char));
      } else {
        ret = convert_from_int32(CAST_FROM_STRING(buf, unsigned char));
      }
      return ret;
    case 2:
      memcpy(buf, str + (*str_i), 2);
      *str_i += 2;
      check_endian(buf, 2, endian);

      if (sign == 1) {
        ret = convert_from_int32(CAST_FROM_STRING(buf, int16_t));
      } else {
        ret = convert_from_int32(CAST_FROM_STRING(buf, uint16_t));
      }
      return ret;
    case 4:
      memcpy(buf, str + (*str_i), 4);
      *str_i += 4;
      check_endian(buf, 4, endian);

      if (sign == 1) {
        ret = convert_from_int32(CAST_FROM_STRING(buf, int32_t));
      } else {
        ret = convert_from_int32(CAST_FROM_STRING(buf, uint32_t));
      }
      return ret;
    case 8:
      memcpy(buf, str + (*str_i), 8);
      *str_i += 8;
      check_endian(buf, 8, endian);

      if (sign == 1) {
        ret = convert_from_int64(CAST_FROM_STRING(buf, int64_t));
      } else {
        ret = convert_from_int64(CAST_FROM_STRING(buf, uint64_t));
      }
      return ret;
    default:
      mrb_raisef(mrb, E_ARGUMENT_ERROR, "Cannot pack a fixnum with size %d!",
                 size);
  }
  /* Not reachable */
  return mrb_nil_value();
}

static int
pack_float(mrb_state* mrb, mrb_value v, int is_double, int endian,
           mrb_value ret_str)
{
  char buf[8];
  int size;
  mrb_float f;

  switch (mrb_type(v)) {
    case MRB_TT_FLOAT:
      f = mrb_float(v);
      break;
    case MRB_TT_FIXNUM:
      f = (mrb_float) mrb_fixnum(v);
      break;
    default:
      {
        mrb_value tmp = mrb_convert_type(mrb, v, MRB_TT_FLOAT, "Float", "to_f");
        f = mrb_float(tmp);
      }
      break;
  }

  if (is_double == 1) {
    *((double*) buf) = (double) f;
    size = sizeof(double);
  } else {
    *((float*) buf) = (float) f;
    size = sizeof(float);
  }

  mrb_str_cat(mrb, ret_str, check_endian(buf, size, endian), size);
  return size;
}

static mrb_value
unpack_float(mrb_state* mrb, int is_double, int endian, char* str, int* str_i)
{
  mrb_value ret;
  char buf[8];

  if (is_double == 1) {
    memcpy(buf, str + (*str_i), sizeof(double));
    *str_i += sizeof(double);
    check_endian(buf, sizeof(double), endian);

    ret = mrb_float_value(CAST_FROM_STRING(buf, double));
  } else {
    memcpy(buf, str + (*str_i), sizeof(float));
    *str_i += sizeof(float);
    check_endian(buf, sizeof(float), endian);

    ret = mrb_float_value(CAST_FROM_STRING(buf, float));
  }
  return ret;
}

static mrb_value
mrb_array_pack(mrb_state* mrb, mrb_value ary)
{
  mrb_value *arr, ret;
  char *tstr_p;
  int arr_len, arr_i, tstr_i, tstr_len;
  struct parse_options opts;

  opts.type = PACK_INTEGER;
  opts.pack_len = opts.is_double = 0;
  opts.size = opts.sign = -1;

  /* Template string */
  mrb_get_args(mrb, "s", &tstr_p, &tstr_len);
  tstr_i = 0;

  /* Array to pack */
  arr = RARRAY_PTR(ary);
  arr_len = RARRAY_LEN(ary);
  arr_i = 0;

  /* Returned packed string */
  ret = mrb_str_new_cstr(mrb, "");

  while ((arr_i < arr_len) && (tstr_i < tstr_len)) {
    parse_option(mrb, tstr_p, tstr_len, &tstr_i, &opts);
    if (opts.pack_len == ALL_ARGUMENT_SIZE) {
      opts.pack_len = arr_len - arr_i;
    }

    while (opts.pack_len > 0) {
      switch (opts.type) {
        case PACK_INTEGER:
          pack_fixnum(mrb, arr[arr_i++], opts.size, opts.sign, opts.endian,
                      ret);
          break;
        case PACK_FLOAT:
          pack_float(mrb, arr[arr_i++], opts.is_double, opts.endian, ret);
          break;
      }
      opts.pack_len--;
    }
  }

  return ret;
}

static mrb_value
mrb_string_unpack(mrb_state* mrb, mrb_value str)
{
  mrb_value ret, unpacked_v;
  char *str_p, *tstr_p;
  int str_i, str_len, tstr_i, tstr_len;
  struct parse_options opts;

  opts.type = PACK_INTEGER;
  opts.pack_len = opts.is_double = 0;
  opts.size = opts.sign = -1;

  mrb_get_args(mrb, "s", &tstr_p, &tstr_len);
  tstr_i = 0;

  str_p = RSTRING_PTR(str);
  str_len = RSTRING_LEN(str);
  str_i = 0;

  ret = mrb_ary_new(mrb);

  while ((str_i < str_len) && (tstr_i < tstr_len)) {
    parse_option(mrb, tstr_p, tstr_len, &tstr_i, &opts);
    if (opts.pack_len == ALL_ARGUMENT_SIZE) {
      opts.pack_len = (str_len - str_i) / opts.size;
    }

    while (opts.pack_len > 0) {
      unpacked_v = mrb_nil_value();
      switch (opts.type) {
        case PACK_INTEGER:
          unpacked_v = unpack_fixnum(mrb, opts.size, opts.sign, opts.endian,
                                     str_p, &str_i);
          break;
        case PACK_FLOAT:
          unpacked_v = unpack_float(mrb, opts.is_double, opts.endian,
                                    str_p, &str_i);
          break;
      }
      if (!mrb_nil_p(unpacked_v)) {
        mrb_ary_push(mrb, ret, unpacked_v);
      }
      opts.pack_len--;
    }
  }

  return ret;
}

void
mrb_mruby_pack_gem_init(mrb_state* mrb)
{
  mrb_define_method(mrb, mrb->array_class, "pack", mrb_array_pack, ARGS_REQ(1));
  mrb_define_method(mrb, mrb->string_class, "unpack", mrb_string_unpack, ARGS_REQ(1));
}
