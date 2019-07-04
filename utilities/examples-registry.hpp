#if __cplusplus > 199711L
#define DELETED = delete
#else
#define DELETED
#endif


class ExamplesRegistry
{
public:
	typedef std::ofstream Output;

	typedef bool(*Callback)(Output& output);

	class Item
	{
	public:
		Item(Callback);
	private:
		Item(const Item&) DELETED;
		friend class ExamplesRegistry;
		Callback callback;
		Item* next;
	};

	static void RunAll();
private:
	static Item* first;
	static Item* last;

	static void RegisterItem(Item* item);

	ExamplesRegistry() DELETED;
	ExamplesRegistry(const ExamplesRegistry&) DELETED;
	ExamplesRegistry(ExamplesRegistry&&) DELETED;
	~ExamplesRegistry() DELETED;
	void* operator=(const ExamplesRegistry&) DELETED;
	void* operator=(ExamplesRegistry&&) DELETED;
};


#undef DELETED