#include <stdio.h>
#include <stdint.h>

#include <mruby.h>
#include <mruby/array.h>
#include <mruby/numeric.h>
#include <mruby/string.h>

#define ROUND_FLOAT(f_) ((f_) >= 0 ? (f_ + 0.5) : (f_ - 0.5))

#define CAST_FROM_STRING(str_p_, str_i_, t_) *((t_*) ((str_p_) + (str_i_)))

enum pack_type {
  PACK_INTEGER = 0,
  PACK_FLOAT
};

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
pack_fixnum(mrb_state* mrb, mrb_value v, int size, int sign, mrb_value ret_str)
{
  char buf[8];

  switch (size) {
    case 1:
      if (sign == 1) {
        *((char*) buf) = (char) convert_to_int32(mrb, v, sign);
      } else {
        *((unsigned char*) buf) = (unsigned char) convert_to_int32(mrb, v, sign);
      }
      mrb_str_cat(mrb, ret_str, buf, 1);
      return 1;
    case 2:
      if (sign == 1) {
        *((int16_t*) buf) = (int16_t) convert_to_int32(mrb, v, sign);
      } else {
        *((uint16_t*) buf) = (uint16_t) convert_to_int32(mrb, v, sign);
      }
      mrb_str_cat(mrb, ret_str, buf, 2);
      return 2;
    case 4:
      if (sign == 1) {
        *((int32_t*) buf) = (int32_t) convert_to_int32(mrb, v, sign);
      } else {
        *((uint32_t*) buf) = (uint32_t) convert_to_int32(mrb, v, sign);
      }
      mrb_str_cat(mrb, ret_str, buf, 4);
      return 4;
    case 8:
      if (sign == 1) {
        *((int64_t*) buf) = (int64_t) convert_to_int64(mrb, v, sign);
      } else {
        *((uint64_t*) buf) = (uint64_t) convert_to_int64(mrb, v, sign);
      }
      mrb_str_cat(mrb, ret_str, buf, 8);
      return 8;
    default:
      mrb_raisef(mrb, E_ARGUMENT_ERROR, "Cannot pack a fixnum with size %d!",
                 size);
  }

  /* Actually this is not reachable. */
  return 0;
}

static mrb_value
unpack_fixnum(mrb_state* mrb, int size, int sign, char* str, int* str_i)
{
  mrb_value ret;
  switch (size) {
    case 1:
      if (sign == 1) {
        ret = convert_from_int32(CAST_FROM_STRING(str, *str_i, char));
      } else {
        ret = convert_from_int32(CAST_FROM_STRING(str, *str_i, unsigned char));
      }
      *str_i += 1;
      return ret;
    case 2:
      if (sign == 1) {
        ret = convert_from_int32(CAST_FROM_STRING(str, *str_i, int16_t));
      } else {
        ret = convert_from_int32(CAST_FROM_STRING(str, *str_i, uint16_t));
      }
      *str_i += 2;
      return ret;
    case 4:
      if (sign == 1) {
        ret = convert_from_int32(CAST_FROM_STRING(str, *str_i, int32_t));
      } else {
        ret = convert_from_int32(CAST_FROM_STRING(str, *str_i, uint32_t));
      }
      *str_i += 4;
      return ret;
    case 8:
      if (sign == 1) {
        ret = convert_from_int64(CAST_FROM_STRING(str, *str_i, int64_t));
      } else {
        ret = convert_from_int64(CAST_FROM_STRING(str, *str_i, uint64_t));
      }
      *str_i += 8;
      return ret;
    default:
      mrb_raisef(mrb, E_ARGUMENT_ERROR, "Cannot pack a fixnum with size %d!",
                 size);
  }
  /* Not reachable */
  return mrb_nil_value();
}

static int
pack_float(mrb_state* mrb, mrb_value v, int is_double, mrb_value ret_str)
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

  mrb_str_cat(mrb, ret_str, buf, size);
  return size;
}

static mrb_value
unpack_float(mrb_state* mrb, int is_double, char* str, int* str_i)
{
  mrb_value ret;
  if (is_double == 1) {
    ret = mrb_float_value(CAST_FROM_STRING(str, *str_i, double));
    *str_i += sizeof(double);
  } else {
    ret = mrb_float_value(CAST_FROM_STRING(str, *str_i, float));
    *str_i += sizeof(float);
  }
  return ret;
}

static mrb_value
mrb_array_pack(mrb_state* mrb, mrb_value ary)
{
  mrb_value *arr, ret;
  char *tstr_p, c;
  int arr_len, arr_i, tstr_i, tstr_len;
  enum pack_type type = PACK_INTEGER;
  int size = -1, sign = -1, pack_len = 0, is_double = 0;

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
    c = tstr_p[tstr_i++];

    switch (c) {
      case 'C':
        pack_len = 1;
        type = PACK_INTEGER;
        sign = 0;
        size = 1;
        break;
      case 'c':
        pack_len = 1;
        type = PACK_INTEGER;
        sign = 1;
        size = 1;
        break;
      case 'S':
        pack_len = 1;
        type = PACK_INTEGER;
        sign = 0;
        size = 2;
        break;
      case 's':
        pack_len = 1;
        type = PACK_INTEGER;
        sign = 1;
        size = 2;
        break;
      case 'L':
        pack_len = 1;
        type = PACK_INTEGER;
        sign = 0;
        size = 4;
        break;
      case 'l':
        pack_len = 1;
        type = PACK_INTEGER;
        sign = 1;
        size = 4;
        break;
      case 'Q':
        pack_len = 1;
        type = PACK_INTEGER;
        sign = 0;
        size = 8;
        break;
      case 'q':
        pack_len = 1;
        type = PACK_INTEGER;
        sign = 1;
        size = 8;
        break;
      case 'D':
      case 'd':
        pack_len = 1;
        type = PACK_FLOAT;
        is_double = 1;
        break;
      case 'F':
      case 'f':
        pack_len = 1;
        type = PACK_FLOAT;
        is_double = 0;
        break;
      case '*':
        /* Uses type, sign, size from last run */
        if (tstr_i == 1) {
          mrb_raise(mrb, E_ARGUMENT_ERROR,
                    "'*' must follow existing directives!");
        }
        pack_len = arr_len - arr_i;
        break;
    }

    while (pack_len > 0) {
      switch (type) {
        case PACK_INTEGER:
          pack_fixnum(mrb, arr[arr_i++], size, sign, ret);
          break;
        case PACK_FLOAT:
          pack_float(mrb, arr[arr_i++], is_double, ret);
          break;
      }
      pack_len--;
    }
  }

  return ret;
}

static mrb_value
mrb_string_unpack(mrb_state* mrb, mrb_value str)
{
  mrb_value ret, unpacked_v;
  char *str_p, *tstr_p, c;
  int str_i, str_len, tstr_i, tstr_len;
  enum pack_type type = PACK_INTEGER;
  int size = -1, sign = -1, pack_len = 0, is_double = 0;

  mrb_get_args(mrb, "s", &tstr_p, &tstr_len);
  tstr_i = 0;

  str_p = RSTRING_PTR(str);
  str_len = RSTRING_LEN(str);
  str_i = 0;

  ret = mrb_ary_new(mrb);

  while ((str_i < str_len) && (tstr_i < tstr_len)) {
    c = tstr_p[tstr_i++];

    switch (c) {
      case 'C':
        pack_len = 1;
        type = PACK_INTEGER;
        sign = 0;
        size = 1;
        break;
      case 'c':
        pack_len = 1;
        type = PACK_INTEGER;
        sign = 1;
        size = 1;
        break;
      case 'S':
        pack_len = 1;
        type = PACK_INTEGER;
        sign = 0;
        size = 2;
        break;
      case 's':
        pack_len = 1;
        type = PACK_INTEGER;
        sign = 1;
        size = 2;
        break;
      case 'L':
        pack_len = 1;
        type = PACK_INTEGER;
        sign = 0;
        size = 4;
        break;
      case 'l':
        pack_len = 1;
        type = PACK_INTEGER;
        sign = 1;
        size = 4;
        break;
      case 'Q':
        pack_len = 1;
        type = PACK_INTEGER;
        sign = 0;
        size = 8;
        break;
      case 'q':
        pack_len = 1;
        type = PACK_INTEGER;
        sign = 1;
        size = 8;
        break;
      case 'D':
      case 'd':
        pack_len = 1;
        type = PACK_FLOAT;
        is_double = 1;
        break;
      case 'F':
      case 'f':
        pack_len = 1;
        type = PACK_FLOAT;
        is_double = 0;
        break;
      case '*':
        /* Uses type, sign, size from last run */
        if (tstr_i == 1) {
          mrb_raise(mrb, E_ARGUMENT_ERROR,
                    "'*' must follow existing directives!");
        }
        pack_len = (str_len - str_i) / size;
        break;
    }

    while (pack_len > 0) {
      unpacked_v = mrb_nil_value();
      switch (type) {
        case PACK_INTEGER:
          unpacked_v = unpack_fixnum(mrb, size, sign, str_p, &str_i);
          break;
        case PACK_FLOAT:
          unpacked_v = unpack_float(mrb, is_double, str_p, &str_i);
          break;
      }
      if (!mrb_nil_p(unpacked_v)) {
        mrb_ary_push(mrb, ret, unpacked_v);
      }
      pack_len--;
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
