#include "examples-registry.hpp"

ExamplesRegistry::Callback ExamplesRegistry::RunAll(Output &output)
{
    for (Item* item = first; item; item = item->next)
    {
        if (! item->callback(output))
        {
            return item->callback;
        }
    }
    return Callback();
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

ExamplesRegistry::Item::Item(Callback callback) 
    : callback(callback), next()
{
    RegisterItem(this);
}

ExamplesRegistry::Item* ExamplesRegistry::first;
ExamplesRegistry::Item* ExamplesRegistry::last;
