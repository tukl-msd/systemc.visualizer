#ifndef VISUALIZE_H
#define VISUALIZE_H
#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include "systemc.h"
#include <regex>
#include <sstream>

class Port
{
    public:
    std::string id;
    std::string name;
    sc_interface * ptr;
    bool input;

    Port(std::string id, std::string name, sc_interface * ptr, bool input)
        : id(id), name(name), ptr(ptr), input(input)
    {
    }
};

class Module
{
    public:
    std::string id;
    std::string name;
    bool topModule;

    Module(std::string id, std::string name, bool top) :
        id(id), name(name), topModule(top)
    {
    }

    Module()
    {
    }

    std::vector<Module> submodules;
    std::vector<Port> ports;
};

class visualize
{
    private:

    ofstream e;

    // Used for proper indention of the DOT file:
    unsigned int level;

    bool debug;

    std::vector<Module> topModules;

    public:

    visualize(bool debug=false) : debug(debug)
    {
        std::string elk = "graph.elk";
        e.open(elk);

        level = 0;

        std::vector<sc_object*> tops = sc_get_top_level_objects();
        for(auto t: tops)
        {
            if(std::string(t->kind()) == "sc_module")
            {
                if(debug)
                {
                    std::cout << t->name() << "("
                              << t->kind() << ")" << std::endl;
                }

                topModules.push_back(Module(t->name(),
                                            t->basename(),
                                            true));

                scanModule(*t, &topModules.back());
            }
        }
    }

    visualize(sc_module &t, bool debug = false) : debug(debug)
    {
        std::cout << "ERROR: Not implementet yet" << std::endl;
    }

    ~visualize()
    {
        generateElk(topModules);
    }

    void scanModule(sc_object &t, Module * currentModule)
    {
        level++;

        for(auto child : t.get_child_objects())
        {
            if(debug)
            {
                indent(std::cout, level);
                std::cout << child->name()
                          << "(" << child->kind() << ")" << std::endl;
            }

            if(std::string(child->kind()) == "sc_module")
            {
                currentModule->submodules.push_back(
                    Module(child->name(),child->basename(),false)
                );

                // Analyze the Child:
                scanModule(*child, & currentModule->submodules.back());
            }
            else // If port was found:
            if(std::string(child->kind()) == "sc_in" ||
               std::string(child->kind()) == "sc_out" ||
               std::string(child->kind()) == "sc_fifo_in" ||
               std::string(child->kind()) == "sc_fifo_out" ||
               std::string(child->kind()) == "tlm_target_socket" ||
               std::string(child->kind()) == "sc_port")
            {
                sc_port_base * optr = dynamic_cast<sc_port_base*>(child);
                sc_interface * pointer = optr->get_interface();

                bool input = std::string(child->kind()) == "sc_in" ||
                             std::string(child->kind()) == "sc_fifo_in" ||
                             std::string(child->kind()) == "tlm_target_socket";

                currentModule->ports.push_back(Port(child->name(),
                                                    child->basename(),
                                                    pointer,
                                                    input));
            }
            else if(std::string(child->kind()) == "tlm_initiator_socket" ||
                    std::string(child->kind()) == "sc_export" )
            {
                // TODO does not work yet
            }
        }

        level--;
    }

    void generateElk(std::vector<Module> modules)
    {
        for(auto module : modules)
        {
            std::cout << module.name << std::endl;

            indent(e,level);
            //e << "node " << module.id << std::endl;
            e << "node " << module.name << std::endl;
            indent(e,level);
            e << "{ " << std::endl;
            indent(e,level+1);
            e << "label \"" << module.name << "\"" << std::endl;

            for(auto port : module.ports)
            {
                std::cout << "    " << port.name << std::endl;
                indent(e,level+1);
                //e << "port " << port.id
                e << "port " << port.name
                  << " { label '" << port.name << "' }" << std::endl;
            }

            level ++;
            generateElk(module.submodules);
            level --;
            indent(e,level);

            e << "}" << std::endl;

            // Draw edges:
            for(auto port : module.ports)
            {
                sc_interface * pointer = port.ptr;
                for(auto module2 : modules)
                {
                    for(auto port2 : module2.ports)
                    {
                        if(port2.ptr == pointer && (port2.id != port.id))
                        {
                            if(port2.input == true && port.input == false)
                            {
                                indent(e,level);
                                e << "edge " << port.id
                                  << " -> " << port2.id << std::endl;
                            }
                        }
                    }
                }
            }
        }
    }

    // Indention acording to the level:
    void indent(std::ostream &o, unsigned int level)
    {
        for(unsigned int i = 0; i < level; i++)
        {
            o << "    ";
        }
    }

};


#endif // VISUALIZE_H
