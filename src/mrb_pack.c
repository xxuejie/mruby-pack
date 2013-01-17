#include <stdint.h>

#include <mruby.h>
#include <mruby/array.h>
#include <mruby/string.h>

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
          tmp = mrb_convert_type(mrb, arr[arr_i++], MRB_TT_FIXNUM,
                                 "Integer", "to_int");
          buf[0] = (unsigned char) mrb_fixnum(tmp);
          mrb_str_cat(mrb, ret, buf, 1);
        }
        break;
      case 'S':
        {
          tmp = mrb_convert_type(mrb, arr[arr_i++], MRB_TT_FIXNUM,
                                 "Integer", "to_int");
          /* native endian */
          *((uint16_t*) buf) = (uint16_t) mrb_fixnum(tmp);
          mrb_str_cat(mrb, ret, buf, 2);
        }
        break;
      case 'L':
        {
          /* TODO: internally, mrb_fixnum is implemented using int, so
           * cannot use all the values from uint32_t.
           * We may choose to use float to emulate bigger ints.
           */
          tmp = mrb_convert_type(mrb, arr[arr_i++], MRB_TT_FIXNUM,
                                 "Integer", "to_int");
          *((uint32_t*) buf) = (uint32_t) mrb_fixnum(tmp);
          mrb_str_cat(mrb, ret, buf, 4);
        }
        break;
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
      case 'S':
        {
          uint16_t v;
          v = *((uint16_t*) (str_p + str_i));
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
