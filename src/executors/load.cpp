#include "global.h"
/**
 * @brief 
 * SYNTAX: LOAD <relation_name> OR LOAD MATRIX <matrix_name>
 */
bool syntacticParseLOAD()
{
    if (tokenizedQuery.size() == 3 && tokenizedQuery[1] == "MATRIX")
    {
        logger.log("syntacticParseLOAD_MATRIX");
        parsedQuery.queryType = LOAD_MATRIX;
        parsedQuery.loadRelationName = tokenizedQuery[2];
        return true;
    }
    logger.log("syntacticParseLOAD");
    if (tokenizedQuery.size() != 2)
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = LOAD;
    parsedQuery.loadRelationName = tokenizedQuery[1];
    return true;
}

bool semanticParseLOAD()
{
    logger.log("semanticParseLOAD");
    if (tableCatalogue.isTable(parsedQuery.loadRelationName))
    {
        cout << "SEMANTIC ERROR: Relation already exists" << endl;
        return false;
    }

    if (!isFileExists(parsedQuery.loadRelationName))
    {
        cout << "SEMANTIC ERROR: Data file doesn't exist" << endl;
        return false;
    }
    return true;
}

void executeLOAD()
{
    logger.log("executeLOAD");

    Table *table = new Table(parsedQuery.loadRelationName);
    if (table->load())
    {
        tableCatalogue.insertTable(table);
        cout << "Loaded Table. Column Count: " << table->columnCount << " Row Count: " << table->rowCount << endl;
    }
    return;
}