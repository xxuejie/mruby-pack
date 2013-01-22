#include <stdio.h>
#include <stdint.h>

#include <mruby.h>
#include <mruby/array.h>
#include <mruby/string.h>

static mrb_value
convert_to_integer(mrb_state* mrb, mrb_value v)
{
  mrb_value i = mrb_check_convert_type(mrb, v, MRB_TT_FIXNUM,
                                       "Integer", "to_int");
  if (mrb_nil_p(i)) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "Cannot convert given value to fixnum!");
  }

  return i;
}

static mrb_value
mrb_array_pack(mrb_state* mrb, mrb_value ary)
{
  mrb_value *arr, ret, tmp;
  char *tstr_p, c, buf[8];
  int arr_len, arr_i, tstr_i, tstr_len;

  mrb_get_args(mrb, "s", &tstr_p, &tstr_len);
  tstr_i = 0;

  arr = RARRAY_PTR(ary);
  arr_len = RARRAY_LEN(ary);
  arr_i = 0;

  ret = mrb_str_new_cstr(mrb, "");

  while ((arr_i < arr_len) && (tstr_i < tstr_len)) {
    c = tstr_p[tstr_i++];

    switch (c) {
      case 'C':
        {
          tmp = convert_to_integer(mrb, arr[arr_i++]);
          buf[0] = (unsigned char) mrb_fixnum(tmp);
          mrb_str_cat(mrb, ret, buf, 1);
        }
        break;
      case 'c':
        {
          tmp = convert_to_integer(mrb, arr[arr_i++]);
          buf[0] = (char) mrb_fixnum(tmp);
          mrb_str_cat(mrb, ret, buf, 1);
        }
        break;
      case 'S':
        {
          tmp = convert_to_integer(mrb, arr[arr_i++]);
          /* native endian */
          *((uint16_t*) buf) = (uint16_t) mrb_fixnum(tmp);
          mrb_str_cat(mrb, ret, buf, 2);
        }
        break;
      case 's':
        {
          tmp = convert_to_integer(mrb, arr[arr_i++]);
          *((int16_t*) buf) = (int16_t) mrb_fixnum(tmp);
          mrb_str_cat(mrb, ret, buf, 2);
        }
        break;
      case 'L':
        {
          /*
           * NOTE: by default mruby use int32_t as the type of mrb_int.
           * As a result, any value greater than 2147483647 will be of
           * float type, and will trigger an error here, even though we
           * are using uint32_t.
           * We may choose to convert float value back to int, but that
           * may bring undefined behavior which will differ on different
           * platforms. Will come back later on this problem.
           */
          tmp = convert_to_integer(mrb, arr[arr_i++]);
          *((uint32_t*) buf) = (uint32_t) mrb_fixnum(tmp);
          mrb_str_cat(mrb, ret, buf, 4);
        }
        break;
      case 'l':
        {
          tmp = convert_to_integer(mrb, arr[arr_i++]);
          *((int32_t*) buf) = (int32_t) mrb_fixnum(tmp);
          mrb_str_cat(mrb, ret, buf, 4);
        }
        break;
#ifdef MRB_INT64
      case 'Q':
        {
          tmp = convert_to_integer(mrb, arr[arr_i++]);
          *((uint64_t*) buf) = (uint64_t) mrb_fixnum(tmp);
          mrb_str_cat(mrb, ret, buf, 8);
        }
        break;
      case 'q':
        {
          tmp = convert_to_integer(mrb, arr[arr_i++]);
          *((int64_t*) buf) = (int64_t) mrb_fixnum(tmp);
          mrb_str_cat(mrb, ret, buf, 8);
        }
        break;
#endif
    }
  }

  return ret;
}

static mrb_value
mrb_string_unpack(mrb_state* mrb, mrb_value str)
{
  mrb_value ret;
  char *str_p, *tstr_p, c;
  int str_i, str_len, tstr_i, tstr_len;

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
        {
          unsigned char v;
          v = *((unsigned char*) (str_p + str_i));
          str_i++;
          mrb_ary_push(mrb, ret, mrb_fixnum_value(v));
        }
        break;
      case 'c':
        {
          char v;
          v = *((char*) (str_p + str_i));
          str_i++;
          mrb_ary_push(mrb, ret, mrb_fixnum_value(v));
        }
        break;
      case 'S':
        {
          uint16_t v;
          v = *((uint16_t*) (str_p + str_i));
          str_i += 2;
          mrb_ary_push(mrb, ret, mrb_fixnum_value(v));
        }
        break;
      case 's':
        {
          int16_t v;
          v = *((int16_t*) (str_p + str_i));
          str_i += 2;
          mrb_ary_push(mrb, ret, mrb_fixnum_value(v));
        }
        break;
      case 'L':
        {
          uint32_t v;
          v = *((uint32_t*) (str_p + str_i));
          str_i += 4;
          mrb_ary_push(mrb, ret, mrb_fixnum_value(v));
        }
        break;
      case 'l':
        {
          int32_t v;
          v = *((int32_t*) (str_p + str_i));
          str_i += 4;
          mrb_ary_push(mrb, ret, mrb_fixnum_value(v));
        }
        break;
#ifdef MRB_INT64
      case 'Q':
        {
          uint64_t v;
          v = *((uint64_t*) (str_p + str_i));
          str_i += 8;
          mrb_ary_push(mrb, ret, mrb_fixnum_value(v));
        }
        break;
      case 'q':
        {
          int64_t v;
          v = *((int64_t*) (str_p + str_i));
          str_i += 8;
          mrb_ary_push(mrb, ret, mrb_fixnum_value(v));
        }
        break;
#endif
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
