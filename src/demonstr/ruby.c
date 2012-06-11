#include "everything.h"

#include <ruby.h>

VALUE
logic_data(void)
{
    int i;
    VALUE array;
    int* args;

    thread_cond_wait(&logic.data_arrived, logic.data);
    args = logic.data.val;

    for (i = 0; i < logic.samples; i++) {
        args[i] = INT2FIX(args[i]);
    }

    array = rb_ary_new4(logic.samples, logic.data.val);
    thread_unlock(logic.data);

    return array;
}

void
demonstr_ruby_run()
{
    VALUE rb_cDemonstr;

    rb_cDemonstr = rb_define_class("Demonstrate", rb_cObject);
    rb_define_singleton_method(rb_cDemonstr, "logic_data", logic_data, 0);
    rb_define_singleton_method(rb_cDemonstr, "fin", (VALUE (*)()) exit, 0);

    rb_f_require(rb_mKernel, rb_str_new2(DEMONSTR_SCRIPT));
    rb_funcall(rb_mKernel, rb_intern("main"), 0);
}

void
demonstr_ruby_init()
{
    int argc = 1;
    char* argv[] = {DEMONSTR_SCRIPT, 0};

    ruby_sysinit(&argc, (char***) &argv);

    RUBY_INIT_STACK;
    ruby_init();
    ruby_script("__voculus__");
}
