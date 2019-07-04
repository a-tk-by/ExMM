ExamplesRegistry::callback ExamplesRegistry::RunAll(ExamplesRegistry::Output &output)
{
	for (ExamplesRegistry::Item* item = first; item; item = item->next)
	{
		if (! item->callback(output))
		{
			return callback;
		}
	}
}

void ExamplesRegistry::RegisterItem(ExamplesRegistry::Item* item)
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

void ExamplesRegistry::Item::Item(Callback callback) : callback(callback)
{
	ExamplesRegistry::RegisterItem(this);
}

ExamplesRegistry::Item* ExamplesRegistry::first;
ExamplesRegistry::Item* ExamplesRegistry::last;
