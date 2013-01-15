#include <mruby.h>

static mrb_value
mrb_array_pack(mrb_state* mrb, mrb_value ary)
{
  /* Not implemented yet! */
  return mrb_nil_value();
}

static mrb_value
mrb_string_unpack(mrb_state* mrb, mrb_value str)
{
  /* Not implemented yet! */
  return mrb_nil_value();
}

void
mrb_mruby_pack_gem_init(mrb_state* mrb)
{
  mrb_define_method(mrb, mrb->array_class, "pack", mrb_array_pack, ARGS_REQ(1));
  mrb_define_method(mrb, mrb->string_class, "unpack", mrb_string_unpack, ARGS_REQ(1));
}
