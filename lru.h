class LinkNode {
public:
    int key;
    int val;
    LinkNode* prev;
    LinkNode* next;
    LinkNode(int _key, int _val): key(_key), val(_val), prev(nullptr), next(nullptr) {};
};

class LRUCache {
private:
    int size = 0;
    int maxSize;
    LinkNode* virHead;
    LinkNode* virTail;
    map<int, LinkNode*> mymap;
public:
    LRUCache(int capacity) {
        virHead = new LinkNode(0,0);
        virTail = new LinkNode(0,0);
        virHead->next = virTail;
        virTail->prev = virHead;
        maxSize = capacity;
    }

    int get(int key) {
        if (mymap[key]) {
            LinkNode* cur = mymap[key];
            cur->prev->next = cur->next;
            cur->next->prev = cur->prev;
            cur->next = virHead->next;
            cur->prev = virHead;
            virHead->next->prev = cur;
            virHead->next = cur;
            return cur->val;
        }
        else return -1;
    }

    void put(int key, int value) {
        if (mymap[key]) {
            LinkNode* cur = mymap[key];
            cur->prev->next = cur->next;
            cur->next->prev = cur->prev;
            cur->next = virHead->next;
            cur->prev = virHead;
            virHead->next->prev = cur;
            virHead->next = cur;
            cur->val = value;
        }
        else {
            LinkNode* newNode = new LinkNode(key, value);
            size++;
            newNode->next = virHead->next;
            newNode->prev = virHead;
            virHead->next->prev = newNode;
            virHead->next = newNode;
            if (size > maxSize) {
                LinkNode* cur = virTail->prev;
                virTail->prev = cur->prev;
                cur->prev->next = virTail;
                cur->prev = nullptr;
                cur->next = nullptr;
                mymap[cur->key] = nullptr;
            }
            mymap[key] = newNode;
        }
    }

};
/**
 * Your LRUCache object will be instantiated and called as such:
 * LRUCache* obj = new LRUCache(capacity);
 * int param_1 = obj->get(key);
 * obj->put(key,value);
 */
