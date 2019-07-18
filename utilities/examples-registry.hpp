#ifndef _EXAMPLE_REGISTRY_H_
#define _EXAMPLE_REGISTRY_H_

#include <iostream>

class ExamplesRegistry
{
public:
    typedef std::ostream Output;

    typedef bool(*Callback)(Output& output);

    class Item
    {
    public:
        Item(Callback);
        Item(const Item&) = delete;
    private:
        friend class ExamplesRegistry;
        Callback callback;
        Item* next;
    };

    static Callback RunAll(Output &output);

    ExamplesRegistry() = delete;
    ExamplesRegistry(const ExamplesRegistry&) = delete;
    ExamplesRegistry(ExamplesRegistry&&) = delete;
    ~ExamplesRegistry() = delete;
    void* operator=(const ExamplesRegistry&) = delete;
    void* operator=(ExamplesRegistry&&) = delete;

private:
    static Item* first;
    static Item* last;

    static void RegisterItem(Item* item);
};

#define EXMM_DEMO(Function) \
    static bool Function(ExamplesRegistry::Output&); \
    static ExamplesRegistry::Item Demo_##Function = Function; \
    static bool Function(ExamplesRegistry::Output& output)

#endif
