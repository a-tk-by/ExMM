#ifndef _EXAMPLE_REGISTRY_H_
#define _EXAMPLE_REGISTRY_H_

#include <iostream>
#include <functional>

class ExamplesRegistry
{
public:
    typedef std::ostream Output;

    struct Callback
    {
        typedef bool(*FunctionPointer)(Output& output);

        const FunctionPointer Function;
        const char* Name;
        
        Callback(FunctionPointer function, const char* name);

        operator bool() const
        {
            return Function != nullptr;
        }

        static Callback Empty;
    };

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

    static bool RunAll(
        Output &output, 
        const std::function<void(const Callback&)>& started,
        const std::function<void(const Callback&)>& success,
        const std::function<void(const Callback&)>& fail
        );

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
    static ExamplesRegistry::Item Demo_##Function = ExamplesRegistry::Callback(Function, #Function);\
    static bool Function(ExamplesRegistry::Output& output)

#endif
