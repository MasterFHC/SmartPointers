int* getNewBlock(int n);
/**
 * Free specified 4096 * n bytes from the memory.
 * param block the pointer to the block
 * param n
 */
void freeBlock(const int* block, int n);
/**
 * Get specified 4096 * n bytes from the memory.
 * param n
 * return the address of the block
 */

#include <vector>
#include <cassert>
#define BLOCK_SIZE (4096 / sizeof(int))

struct MEM_STRIP
{
    int start;
    int length;
    bool occupy;
    struct BLOCK *at_block; // which block is it at
    MEM_STRIP(int st, int le, bool oc, struct BLOCK *bl)
    {
        start = st, length = le, occupy = oc, at_block = bl;
    }
};
struct BLOCK
{
    int *block_pos;                  // start position of this block
    int max_cap;                     // max_capacity(n*blocksize)
    std::vector<MEM_STRIP *> memory; // available memory strips
    BLOCK(int *bp = nullptr, int mc = 0)
    {
        block_pos = bp;
        max_cap = mc;
        memory.push_back(new MEM_STRIP(0, max_cap, false, this));
    }
};

// include whatever you want

class Allocator
{
public:
    Allocator()
    {
        used_block = 0;
        // TODO： constructor 构造函数
    }

    ~Allocator()
    {
        // TODO： destructor 析构函数
        // Please note that even if some of data is not deallocated explicitly,
        // you should free them anyway.
        // 注意，即使此时仍然有没有被 deallocate 的数据，你也要将这些内存释放。
        for (int i = 0; i < used_block; i++)
        {
            BLOCK *now_block = block[i];
            for (int j = 0; j < now_block->memory.size(); j++)
            {
                delete now_block->memory[j];
            }
            now_block->memory.clear();
            freeBlock(now_block->block_pos, now_block->max_cap / BLOCK_SIZE);
            delete now_block;
        }
        block.clear();
    }

    /**
     * Allocate a sequence of memory space of n int.
     * param n
     * return the pointer to the allocated memory space
     */
    int *allocate(int n)
    {
        MEM_STRIP *target_memory = sniff_space(n);
        if (target_memory == nullptr)
        {
            BLOCK *new_block = new BLOCK(getNewBlock(getNeedBlocks(n)), getNeedBlocks(n) * BLOCK_SIZE);
            target_memory = new_block->memory[0];
            block.push_back(new_block);
            used_block++;
        }
        BLOCK *target_block = target_memory->at_block;
        int *ret = target_block->block_pos + target_memory->start;
        target_block->memory.push_back(new MEM_STRIP(target_memory->start, n, true, target_block));
        target_block->memory.push_back(new MEM_STRIP(target_memory->start + n, target_memory->length - n, false, target_block));
        target_block->memory.erase(find_index_strip(target_block, target_memory));
        delete target_memory;
        return ret;
    }

    /**
     * Deallocate the memory that is allocated by the allocate member
     * function. If n is not the number that is called when allocating,
     * the behaviour is undefined.
     */
    void deallocate(int *pointer, int n)
    {
        MEM_STRIP *target_memory = look_for(pointer);
        target_memory->occupy=false;
        BLOCK *target_block = target_memory->at_block;
        MEM_STRIP *pre_memory = nullptr, *nxt_memory = nullptr;
        for (int i = 0; i < target_block->memory.size(); i++)
        { // look for neighbors
            MEM_STRIP *now_strip = target_block->memory[i];
            if (now_strip->start + now_strip->length == target_memory->start and now_strip->occupy == false)
                pre_memory = now_strip;
            else if (now_strip->start == target_memory->start + target_memory->length and now_strip->occupy == false)
                nxt_memory = now_strip;
        }

        if (pre_memory != nullptr)
        { // can merge with the  previous strip
            MEM_STRIP *new_strip = new MEM_STRIP(pre_memory->start, pre_memory->length + target_memory->length, target_memory->occupy, target_block);
            target_block->memory.erase(find_index_strip(target_block, pre_memory));
            target_block->memory.erase(find_index_strip(target_block, target_memory));
            target_block->memory.push_back(new_strip);
            delete pre_memory;
            delete target_memory;
            target_memory = new_strip;
        }
        if (nxt_memory != nullptr)
        { // can merge with the next strip
            MEM_STRIP *new_strip = new MEM_STRIP(target_memory->start, nxt_memory->length + target_memory->length, target_memory->occupy, target_block);
            target_block->memory.erase(find_index_strip(target_block, nxt_memory));
            target_block->memory.erase(find_index_strip(target_block, target_memory));
            target_block->memory.push_back(new_strip);
            delete nxt_memory;
            delete target_memory;
            target_memory = new_strip;
        }
        if (target_block->memory.size() == 1 and target_block->memory[0]->length == target_block->memory.size() and target_block->memory[0]->occupy == false)
        { // this block is not used
            block.erase(find_index_block(target_block));
            used_block--;
            delete target_memory;
            delete target_block;
            freeBlock(target_block->block_pos, n);
        }
    }

private:
    int used_block = 0;
    std::vector<BLOCK *> block;
    inline int getNeedBlocks(int n) { return (n - 1) / BLOCK_SIZE + 1; }
    MEM_STRIP *look_for(int *pointer)
    {
        for (int i = 0; i < used_block; i++)
            for (int j = 0; j < block[i]->memory.size(); j++)
                if (block[i]->block_pos + block[i]->memory[j]->start == pointer)
                    return block[i]->memory[j];
        assert(0);
    }
    std::vector<MEM_STRIP *>::iterator find_index_strip(BLOCK *cur_block, MEM_STRIP *cur_strip)
    {
        for (auto it = cur_block->memory.begin(); it != cur_block->memory.end(); it++)
        {
            if (*it == cur_strip)
                return it;
        }
        assert(0);
    }
    std::vector<BLOCK *>::iterator find_index_block(BLOCK *cur_block)
    {
        for (auto it = block.begin(); it != block.end(); it++)
        {
            if (*it == cur_block)
                return it;
        }
        assert(0);
    }
    MEM_STRIP *sniff_space(int want_size)
    { // size wanted
        MEM_STRIP *candidate = nullptr;
        int min_length = (int)1e9;
        for (int i = 0; i < used_block; i++)
        {
            for (int j = 0; j < block[i]->memory.size(); j++)
            {
                MEM_STRIP *now_strip = block[i]->memory[j];
                if (now_strip->occupy == true)
                    continue;
                if (now_strip->length >= want_size and now_strip->length < min_length)
                {
                    candidate = now_strip;
                    min_length = now_strip->length;
                }
            }
        }
        return candidate;
    }
};