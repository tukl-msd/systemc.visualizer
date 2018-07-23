#include "systemc.h"
#include "visualize.h"

SC_MODULE(A)
{
    sc_out<int> out;

    SC_CTOR(A) : out("out")
    {

    }
};

SC_MODULE(B)
{
    sc_in<int> in;

    SC_CTOR(B) : in ("in")
    {

    }
};

SC_MODULE(top)
{
       A a;
       B b;

       sc_signal<int> sig;

       SC_CTOR(top) : a("a"), b("b")
       {
           a.out(sig);
           b.in(sig);
       }
};

int sc_main(int argc, char *argv[])
{
    top t("top");
    visualize v; // Just instantiate here

    sc_start();
}
