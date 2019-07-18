#include "examples-registry.hpp"

bool ExamplesRegistry::RunAll(
    Output &output,
    const std::function<void(const Callback&)>& success,
    const std::function<void(const Callback&)>& fail
)
{
    bool somethingFailed = false;
    for (Item* item = first; item; item = item->next)
    {
        if (item->callback.Function(output))
        {
            success(item->callback);
        }
        else
        {
            fail(item->callback);
            somethingFailed = true;
        }
    }
    return !somethingFailed;
}

void ExamplesRegistry::RegisterItem(Item* item)
{
    if (first)
    {
        last->next = item;
        last = item;
    }
    else
    {
        first = last = item;
    }
}

ExamplesRegistry::Callback::Callback(FunctionPointer function, const char* name): Function(function), Name(name)
{
}

ExamplesRegistry::Item::Item(Callback callback) 
    : callback(callback), next()
{
    RegisterItem(this);
}

ExamplesRegistry::Item* ExamplesRegistry::first;
ExamplesRegistry::Item* ExamplesRegistry::last;

