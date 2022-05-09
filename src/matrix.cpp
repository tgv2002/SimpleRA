#include "global.h"
#include <memory>
#include <queue>
#include <utility>

/**
 * @brief Construct a new Matrix:: Matrix object
 *
 */
Matrix::Matrix()
{
    logger.log("Matrix::Matrix");
}

/**
 * @brief Construct a new Matrix:: Matrix object used in the case where the data
 * file is available and LOAD command has been called. This command should be
 * followed by calling the load function;
 *
 * @param matrixName 
 */
Matrix::Matrix(string matrixName)
{
    logger.log("Matrix::Matrix");
    this->sourceFileName = "../data/" + matrixName + ".csv";
    this->matrixName = matrixName;
}

/**
 * @brief The load function is used when the LOAD command is encountered. It
 * reads data from the source file, splits it into blocks.
 *
 * @return true if the matrix has been successfully loaded 
 * @return false if an error occurred 
 */
bool Matrix::load()
{
    logger.log("Matrix::load");
    this->getDimensionAndSparsityDetails();
    return this->blockify();
}

/**
 * @brief This function performs in-place transpose to the 
 * given matrix
 */
void Matrix::transpose()
{
    bufferManager.pages.clear();
    // Cursor cursor(this->matrixName, 0, 1, this->isSparse);
    // vector<int> current_element;
    // current_element = cursor.getElementAtIndex(4);
    // logger.log("H_____");
    // logger.log(current_element[0]);
    // cursor =  Cursor(this->matrixName, 3, 1, this->isSparse);
    // current_element = cursor.getElementAtIndex(4);
    // logger.log(current_element[0]);
    logger.log("---------------------");
    // Cursor cursor(this->matrixName, 0, 1, this->isSparse);
    // Page one(this->matrixName, 0, 1, this->isSparse);
    // deque<Page *> pages;
    // pages.push_back(new Page(this->matrixName, 0, 1, this->isSparse));
    deque<std::shared_ptr<Page>> pages;
    pages.push_back(std::make_shared<Page>(this->matrixName, 0, 1, this->isSparse));
    vector<int> current_element;
    int element_ind = -1;
    int el_row, el_col, swap_row, swap_col, swap_ind;
    int lo, hi, mid;
    int block_cnt = -1;
    // Page *firstBlock;
    // Page *secondBlock;
    std::shared_ptr<Page> firstBlock = pages.front(); // Maybe remove this initialization. Probably not required. Have to check sparse before removing.
    std::shared_ptr<Page> secondBlock;

    if(!this->isSparse)
    {
        while(true)
        {
            element_ind++;
            if(element_ind == this->n*this->n){
                while(!pages.empty()){
                    pages.front()->writePage();
                    pages.pop_front();
                }
                break;
            }
            if(element_ind % this->maxElementsPerBlock == 0){
                block_cnt++;
            }
            string first_block_name = bufferManager.getPageName(this->matrixName, block_cnt);

            el_row = element_ind/this->n;
            el_col = element_ind % this->n;
            if(el_row >= el_col)
            {
                continue;
            }
            swap_row = el_col;
            swap_col = el_row;
            swap_ind = swap_row * this->n + swap_col;
            // Binary search to find the page number.
            lo = 0, hi = this->lastCellNumberInBlock.size()-1;
            while(lo < hi){
                mid = (lo + hi)/2;
                if (this->lastCellNumberInBlock[mid] < swap_ind){
                    lo = mid+1;
                } else {
                    hi = mid;
                }
            }
            string second_block_name = bufferManager.getPageName(this->matrixName, lo);

            // if(element_ind == 650) // (0, 299) SWAPPED WITH THIS (299, 0)
            // {
            //     logger.log(this->maxElementsPerBlock);
            //     logger.log(element_ind);
            //     logger.log(swap_ind);
            //     logger.log(lo);
            //     if(lo != 0){
            //         swap_ind = swap_ind - this->lastCellNumberInBlock[lo-1] - 1;
            //     }
            //     logger.log(swap_ind);
            //     break;
            // }
            if(lo != 0){
                swap_ind = swap_ind - this->lastCellNumberInBlock[lo-1] - 1;
            }
            logger.log(this->maxElementsPerBlock);
            logger.log(element_ind);
            logger.log(swap_ind);
            logger.log(lo);
        // Warning:: Make sure to free the memory while popping.
            if(pages.front()->pageName == first_block_name){
                firstBlock = pages.front();
            }else{
                pages.front()->writePage();
                pages.pop_front();
                if(pages.front()->pageName == first_block_name){
                    firstBlock = pages.front();
                }else{
                    // pages.push_front(new Page(this->matrixName, block_cnt, 1, this->isSparse));
                    pages.push_front(std::make_shared<Page>(this->matrixName, block_cnt, 1, this->isSparse));
                    firstBlock = pages.front();
                }
            }

            if(pages.front()->pageName == second_block_name){
                secondBlock = pages.front();
            }else if(pages.back()->pageName == second_block_name){
                secondBlock = pages.back();
            }else{
                if(pages.size()>=2){
                    pages.back()->writePage();
                    pages.pop_back();
                }
                // pages.push_back(new Page(this->matrixName, lo, 1, this->isSparse));
                pages.push_back(std::make_shared<Page>(this->matrixName, lo, 1, this->isSparse));
                secondBlock = pages.back();
            }
            int ind_one = element_ind % this->maxElementsPerBlock;
            int val_one = firstBlock->all_elements[ind_one];
            int val_two = secondBlock->all_elements[swap_ind];
            firstBlock->all_elements[ind_one] = val_two;
            secondBlock->all_elements[swap_ind] = val_one;
        }
    }
    else
    {
        block_cnt = 0;
        int total_blocks = this->lastCellNumberInBlock.size(); // Total number of blocks
        while(true)
        {
            element_ind++;
            if(element_ind == this->n*this->n){
                while(!pages.empty()){
                    pages.front()->writePage();
                    pages.pop_front();
                }
                break;
            }
            
            if(block_cnt+1 != total_blocks)
            {
                if(this->lastCellNumberInBlock[block_cnt]+1 == element_ind){
                    block_cnt++;
                    pages.front()->writePage();
                    int x = pages.front()->non_zero_elements.size();
                    pages.pop_front();
                    if(!pages.empty() && pages.front()->pageName == bufferManager.getPageName(this->matrixName, block_cnt)){
                        
                    }else{
                        pages.push_front(std::make_shared<Page>(this->matrixName, block_cnt, 1, this->isSparse));
                    }
                }
            }

            firstBlock = pages.front();

            el_row = element_ind/this->n;
            el_col = element_ind % this->n;
            if(el_row >= el_col)
            {
                continue;
            }

            swap_row = el_col;
            swap_col = el_row;
            swap_ind = swap_row * this->n + swap_col;

            lo = 0, hi = this->lastCellNumberInBlock.size()-1;
            while(lo < hi){
                mid = (lo + hi)/2;
                if (this->lastCellNumberInBlock[mid] < swap_ind){
                    lo = mid+1;
                } else {
                    hi = mid;
                }
            }
            string second_block_name = bufferManager.getPageName(this->matrixName, lo);
            logger.log(second_block_name);

            if(pages.front()->pageName == second_block_name){
                secondBlock = pages.front();
                logger.log("YES");
            }else if(pages.back()->pageName == second_block_name){
                secondBlock = pages.back();
            }else{
                if(pages.size()>=2){
                    pages.back()->writePage();
                    pages.pop_back();
                }
                pages.push_back(std::make_shared<Page>(this->matrixName, lo, 1, this->isSparse));
                secondBlock = pages.back();
            }
            int val_one = 0;
            int val_two = 0;
            auto tmp_it = firstBlock->non_zero_elements.find(element_ind);
            if(tmp_it != firstBlock->non_zero_elements.end()){
                val_one = tmp_it->second;
            }
            tmp_it = secondBlock->non_zero_elements.find(swap_ind);
            if(tmp_it != secondBlock->non_zero_elements.end()){
                val_two = tmp_it->second;
            }
            logger.log("================");
            logger.log(element_ind);
            logger.log(swap_ind);
            logger.log(second_block_name);
            logger.log(val_one);
            logger.log(val_two);
            // logger.log(firstBlock->nz_itr->first);
            // logger.log(firstBlock->nz_itr->second);
            logger.log("================");
            if(val_one != 0 && val_two != 0){
                logger.log("--------");
                logger.log(val_one);
                logger.log(val_two);
                logger.log("------------");
                tmp_it->second = val_one;
                firstBlock->non_zero_elements[element_ind] = val_two;
            }

            if(val_one != 0 && val_two == 0)
            {
                auto nodeHandler = firstBlock->non_zero_elements.extract(element_ind);
                nodeHandler.key() = swap_ind;
                firstBlock->non_zero_elements.insert(std::move(nodeHandler));
            }

            if(val_one == 0 && val_two != 0)
            {
                auto nodeHandler = secondBlock->non_zero_elements.extract(swap_ind);
                nodeHandler.key() = element_ind;
                secondBlock->non_zero_elements.insert(std::move(nodeHandler));
            }
        }

        // {cell_number, {value, block_num}}
        // std::priority_queue< std::pair<int, std::pair<int, int> >, std::vector< std::pair<int, std::pair<int, int>> >, std::greater< std::pair<int, std::pair<int, int>> > > pq; 
        std::set<int> least_element;
        std::map<int, std::pair<int, int>> val_map;
        // std::priority_queue<std::pair<int, int>, std::vector<std::pair<int, int>>, std::greater<std::pair<int, int>> > free_blocks;
        std::set< std::pair<int, int> , greater<std::pair<int, int>>> free_blocks; // {free size, block_number}


        // Initializing free_blocks
        int tmp = 0;
        for(auto c: this->elementsPerBlockCount)
        {
            free_blocks.insert(std::make_pair(this->maxElementsPerBlock - c, tmp));
            tmp++;
        }

        firstBlock = nullptr;
        for(int cur_block = 0; cur_block < total_blocks-1;  cur_block++)
        {
            firstBlock = std::make_shared<Page>(this->matrixName, cur_block, 1, this->isSparse);
            // for(auto it = firstBlock->non_zero_elements.begin(); it != firstBlock->non_zero_elements.end(); it++)
            // {
            //     pq.push(std::make_pair(it->first, std::make_pair(it->second, cur_block)));
            // }
            std::map<int, int> mp_copy(firstBlock->non_zero_elements);

            // for(auto it = mp_copy.begin(); it != mp_copy.end(); it++){
            //     pq.
            // }

            firstBlock->non_zero_elements.clear(); // Clear the current block's map
            if(cur_block == 0)
            {
                auto it = mp_copy.begin();
                if(it != mp_copy.end()){
                    // pq.insert(std::make_pair(it->first, std::make_pair(it->second, cur_block)));
                    least_element.insert(it->first);
                    val_map[it->first] = std::make_pair(it->second, cur_block);
                }
                
                for(int t_block = cur_block+1; t_block < total_blocks; t_block++)
                {
                    secondBlock = std::make_shared<Page>(this->matrixName, t_block, 1, this->isSparse);
                    if(secondBlock->non_zero_elements.begin() != secondBlock->non_zero_elements.end())
                    {
                        it = secondBlock->non_zero_elements.begin();
                        // pq.insert(std::make_pair(it->first, std::make_pair(it->second, t_block)));
                        // // secondBlock->non_zero_elements.erase(it);
                        // // this->elementsPerBlockCount[t_block]--;
                        // secondBlock->writePage();
                        least_element.insert(it->first);
                        val_map[it->first] = std::make_pair(it->second, t_block);
                    }
                }
            }

            int lim = this->maxElementsPerBlock;
            auto cur_it = least_element.begin();
            int cur = *cur_it;
            while(lim--)
            {
                cur_it = least_element.begin();
                cur = *cur_it;
                int val_now = val_map[cur].first;
                int block_now = val_map[cur].second;

                if(block_now > cur_block){
                    secondBlock = std::make_shared<Page>(this->matrixName, block_now, 1, this->isSparse);
                    secondBlock->non_zero_elements.erase(secondBlock->non_zero_elements.begin());
                    
                    free_blocks.erase(std::make_pair(this->maxElementsPerBlock - this->elementsPerBlockCount[block_now], block_now));
                    this->elementsPerBlockCount[block_now]--;
                    free_blocks.insert(std::make_pair(this->maxElementsPerBlock - this->elementsPerBlockCount[block_now], block_now));

                    auto it = secondBlock->non_zero_elements.begin();
                    if(it != secondBlock->non_zero_elements.end())
                    {
                        // pq.push(std::make_pair(it->first, std::make_pair(it->second, cur.second.second)));
                        // secondBlock->writePage();
                        least_element.insert(it->first);
                        val_map[it->first] = std::make_pair(it->second, block_now);
                    }
                    secondBlock->writePage();
                }
                else if(block_now == cur_block)
                {
                    mp_copy.erase(cur);

                    free_blocks.erase(std::make_pair(this->maxElementsPerBlock - this->elementsPerBlockCount[block_now], block_now));
                    this->elementsPerBlockCount[block_now]--;
                    free_blocks.insert(std::make_pair(this->maxElementsPerBlock - this->elementsPerBlockCount[block_now], block_now));

                    auto it = mp_copy.begin();
                    if(it != mp_copy.end()){    
                        // pq.push(std::make_pair(it->first, std::make_pair(it->second, cur_block)));
                        // // mp_copy.erase(it);
                        least_element.insert(it->first);
                        val_map[it->first] = std::make_pair(it->second, block_now);
                    }
                }
                firstBlock->non_zero_elements[cur] = val_now;
                val_map.erase(cur);
                least_element.erase(cur_it);
            }

            this->elementsPerBlockCount[cur_block] = firstBlock->non_zero_elements.size();
            this->lastCellNumberInBlock[cur_block] = cur;
            firstBlock->writePage();
            firstBlock = nullptr;

            // Redistribute
            if(mp_copy.begin()!=mp_copy.end()){
                auto it = mp_copy.begin();
                least_element.erase(it->first);
                val_map.erase(it->first);

                int tot_size = mp_copy.size();
                int redistributed = 0;
                int cur_block_left = 0;
                int nxt_block = -1;
                while(true)
                {
                    if(tot_size == redistributed){
                        this->elementsPerBlockCount[nxt_block] = this->maxElementsPerBlock - cur_block_left;
                        free_blocks.insert(std::make_pair(this->maxElementsPerBlock - this->elementsPerBlockCount[nxt_block], nxt_block));
                        auto tmpit = firstBlock->non_zero_elements.begin();
                        least_element.insert(tmpit->first);
                        val_map[tmpit->first] = std::make_pair(tmpit->second, nxt_block);
                        firstBlock->writePage();
                        break;
                    }
                    if(cur_block_left == 0)
                    {
                        if(nxt_block != -1){
                            this->elementsPerBlockCount[nxt_block] = this->maxElementsPerBlock - cur_block_left;
                            free_blocks.insert(std::make_pair(this->maxElementsPerBlock - this->elementsPerBlockCount[nxt_block], nxt_block));
                            auto tmpit = firstBlock->non_zero_elements.begin();
                            least_element.insert(tmpit->first);
                            val_map[tmpit->first] = std::make_pair(tmpit->second, nxt_block);
                            firstBlock->writePage();
                        }
                        while(!free_blocks.empty() && free_blocks.begin()->second <= cur_block)
                        {
                            free_blocks.erase(free_blocks.begin());
                        }
                        cur_block_left = free_blocks.begin()->first;
                        nxt_block = free_blocks.begin()->second;
                        free_blocks.erase(free_blocks.begin());

                        firstBlock =  std::make_shared<Page>(this->matrixName, nxt_block, 1, this->isSparse);

                        auto tmpit = firstBlock->non_zero_elements.begin();
                        least_element.erase(tmpit->first);
                        val_map.erase(tmpit->first);
                    }
                    firstBlock->non_zero_elements[it->first] = it->second;
                    it++;
                    redistributed++;
                    cur_block_left--;
                }
            }

        }

        secondBlock = std::make_shared<Page>(this->matrixName, total_blocks-1, 1, this->isSparse);

        // while(!pq.empty()){
        //     auto cur = pq.top();
        //     pq.pop();
        //     secondBlock->non_zero_elements[cur.first] = cur.second.first;
        // }
        // for(auto c: least_element)
        // {
        //     secondBlock->
        // }

        // Redistribution completely makes the final page.
        auto it = secondBlock->non_zero_elements.rbegin();
        if(it != secondBlock->non_zero_elements.rend()){
            this->lastCellNumberInBlock[total_blocks-1] = it->first;
        }
        this->elementsPerBlockCount[total_blocks-1] = secondBlock->non_zero_elements.size();
        secondBlock->writePage();
    }
}

/**
 * @brief This function obtains dimension of the matrix and also obtains info
 * about sparsity of the matrix
 */
void Matrix::getDimensionAndSparsityDetails()
{
    logger.log("Matrix::getDimensionAndSparsityDetails");
    ifstream fin(this->sourceFileName, ios::in);
    string line;
    getline(fin, line);
    string word;
    stringstream s(line);
    while (getline(s, word, ','))
    {
        word.erase(std::remove_if(word.begin(), word.end(), ::isspace), word.end());
        this->n++;
    }
    this->checkSparsity();
    int tuple_size = 1;
    if(this->isSparse)
        tuple_size = 2;
    this->maxElementsPerBlock = (uint)((BLOCK_SIZE * 1000) / (sizeof(int) * tuple_size));
}

/**
 * @brief This function traverses through the matrix and check if it is sparse 
 */
void Matrix::checkSparsity()
{
    logger.log("Matrix::checkSparsity");
    ifstream fin(this->sourceFileName, ios::in);
    string line, word;
    int zeroes = 0;
    while (getline(fin, line))
    {
        stringstream s(line);
        for (int i = 0; i < this->n; i++)
        {
            if (!getline(s, word, ','))
            {
                this->isSparse = false;
                return;
            }
            if (stoi(word) == 0)
                zeroes++;
        }
    }
    if((5 * zeroes) >= (3 * this->n * this->n))
        this->isSparse = true;
    else
        this->isSparse = false;
}

/**
 * @brief This function splits the matrix into elements and stores them 
 * in multiple files of one block size. 
 *
 * @return true if successfully blockified
 * @return false otherwise
 */
bool Matrix::blockify()
{
    logger.log("Matrix::blockify");
    ifstream fin(this->sourceFileName, ios::in);
    string line, word;
    uint pageCounter = 0, lastElement = 0, rowCount = 0, columnsCount = 0;
    map<int, int> nonZeroElementsInPage;
    vector<int> elementsInPage;
    while (getline(fin, line))
    {
        stringstream s(line);
        while(getline(s, word, ','))
        {
            columnsCount++;
            if(!this->isSparse)
            {
                elementsInPage.emplace_back(stoi(word));
                pageCounter++;
                if (pageCounter == this->maxElementsPerBlock)
                {
                    bufferManager.writePage(this->matrixName, this->blockCount, elementsInPage, pageCounter);
                    this->blockCount++;
                    this->elementsPerBlockCount.emplace_back(pageCounter);
                    this->lastCellNumberInBlock.emplace_back(lastElement);
                    pageCounter = 0;
                    elementsInPage.clear();
                }
            }
            else
            {
                if(stoi(word) != 0)
                {
                    nonZeroElementsInPage.insert(make_pair(lastElement, stoi(word)));
                    pageCounter++;
                    if (pageCounter == this->maxElementsPerBlock)
                    {
                        bufferManager.writePage(this->matrixName, this->blockCount, nonZeroElementsInPage, pageCounter);
                        this->blockCount++;
                        this->elementsPerBlockCount.emplace_back(pageCounter);
                        this->lastCellNumberInBlock.emplace_back(lastElement);
                        pageCounter = 0;
                        nonZeroElementsInPage.clear();
                    }
                }
            }
            lastElement++;
        }
        if(columnsCount != this->n)
            return false;
        rowCount++;
        columnsCount = 0;
    }
    if (pageCounter)
    {
        if(this->isSparse)
            bufferManager.writePage(this->matrixName, this->blockCount, nonZeroElementsInPage, pageCounter);
        else
            bufferManager.writePage(this->matrixName, this->blockCount, elementsInPage, pageCounter);
        this->blockCount++;
        this->elementsPerBlockCount.emplace_back(pageCounter);
        this->lastCellNumberInBlock.emplace_back(lastElement);
        lastElement++;
        pageCounter = 0;
        nonZeroElementsInPage.clear();
        elementsInPage.clear();
    }
    if(rowCount != this->n)
        return false;
    return true;
}

/**
 * @brief Function prints the first few rows of the matrix. If the matrix contains
 * more rows than PRINT_COUNT, exactly PRINT_COUNT rows are printed, else all
 * the rows are printed.
 *
 */
void Matrix::print()
{
    logger.log("Matrix::print");
    uint count = min((long long)PRINT_COUNT, (long long)this->n);
    Cursor cursor(this->matrixName, 0, 1, this->isSparse);
    vector<int> current_row;
    vector<int> current_element;
    int previous_element = -1, elementNumber = 0, rows_done = 0;
    if(!this->isSparse)
    {
        while(true)
        {
            current_element = cursor.getNext();
            if(current_element.empty())
                break;
            current_row.emplace_back(current_element[0]);
            if(current_row.size() % this->n == 0)
            {
                this->writeRow(current_row, cout);
                rows_done++;
                current_row.clear();
                if(rows_done == count)
                    break;
            }
        }
    }
    else
    {
        while(true)
        {
            current_element = cursor.getNext();
            if(current_element.empty())
                break;
            elementNumber = current_element[0];
            for(int i=previous_element+1;i<elementNumber;i++)
            {
                current_row.emplace_back(0);
                if(current_row.size() % this->n == 0)
                {
                    this->writeRow(current_row, cout);
                    rows_done++;
                    current_row.clear();
                    if(rows_done == count)
                        break;
                }                
            }
            if(rows_done == count)
                break;
            current_row.emplace_back(current_element[1]);
            if(current_row.size() % this->n == 0)
            {
                this->writeRow(current_row, cout);
                rows_done++;
                current_row.clear();
                if(rows_done == count)
                    break;
            }
            previous_element = elementNumber;
        }
        if(rows_done < count)
        {
            for(int i=previous_element+1;i<this->n*this->n;i++)
            {
                current_row.emplace_back(0);
                if(current_row.size() % this->n == 0)
                {
                    this->writeRow(current_row, cout);
                    rows_done++;
                    current_row.clear();
                    if(rows_done == count)
                        break;
                }            
            }
        }
    }
    printRowCount(this->n);
}

/**
 * @brief This function returns one row of the matrix using the cursor object. It
 * returns an empty row is all rows have been read.
 *
 * @param cursor 
 * @return vector<int> 
 */
void Matrix::getNextPage(Cursor *cursor)
{
    logger.log("Matrix::getNext");
    if (cursor->pageIndex < this->blockCount - 1)
    {
        cursor->nextPage(cursor->pageIndex+1);
    }
}

/**
 * @brief called when EXPORT command is invoked to move source file to "data"
 * folder.
 *
 */
void Matrix::makePermanent()
{
    logger.log("Matrix::makePermanent");
    if(!this->isPermanent())
        bufferManager.deleteFile(this->sourceFileName);
    string newSourceFile = "../data/" + this->matrixName + ".csv";
    ofstream fout(newSourceFile, ios::out);
    Cursor cursor(this->matrixName, 0, 1, this->isSparse);
    vector<int> current_row;
    vector<int> current_element;
    int previous_element = -1, elementNumber = 0;
    if(!this->isSparse)
    {
        while(true)
        {
            current_element = cursor.getNext();
            if(current_element.empty())
                break;
            current_row.emplace_back(current_element[0]);
            if(current_row.size() % this->n == 0)
            {
                this->writeRow(current_row, fout);
                current_row.clear();
            }
        }
    }
    else
    {
        while(true)
        {
            current_element = cursor.getNext();
            if(current_element.empty())
                break;
            elementNumber = current_element[0];
            for(int i=previous_element+1;i<elementNumber;i++)
            {
                current_row.emplace_back(0);
                if(current_row.size() % this->n == 0)
                {
                    this->writeRow(current_row, fout);
                    current_row.clear();
                }                
            }
            current_row.emplace_back(current_element[1]);
            if(current_row.size() % this->n == 0)
            {
                this->writeRow(current_row, fout);
                current_row.clear();
            }
            previous_element = elementNumber;
        }
        for(int i=previous_element+1;i<this->n*this->n;i++)
        {
            current_row.emplace_back(0);
            if(current_row.size() % this->n == 0)
            {
                this->writeRow(current_row, fout);
                current_row.clear();
            }
        }
    }   
    fout.close();
}

/**
 * @brief Function to check if matrix is already exported
 *
 * @return true if exported
 * @return false otherwise
 */
bool Matrix::isPermanent()
{
    logger.log("Matrix::isPermanent");
    if (this->sourceFileName == "../data/" + this->matrixName + ".csv")
        return true;
    return false;
}

/**
 * @brief The unload function removes the matrix from the database by deleting
 * all temporary files created as part of this matrix
 *
 */
void Matrix::unload(){
    logger.log("Matrix::~unload");
    for (int pageCounter = 0; pageCounter < this->blockCount; pageCounter++)
        bufferManager.deleteFile(this->matrixName, pageCounter);
    if (!isPermanent())
        bufferManager.deleteFile(this->sourceFileName);
}

/**
 * @brief Function that returns a cursor that reads rows from this matrix
 * 
 * @return Cursor 
 */
Cursor Matrix::getCursor()
{
    logger.log("Matrix::getCursor");
    Cursor cursor(this->matrixName, 0, 1, this->isSparse);
    return cursor;
}
