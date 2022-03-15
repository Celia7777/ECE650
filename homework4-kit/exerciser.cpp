#include "exerciser.h"

void exercise(connection *C)
{
    // query1(C, 1, 35, 40, 0, 35, 40, 0, 15, 20, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    // query1(C, 0, 35, 40, 1, 20, 30, 0, 15, 20, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    query1(C, 1, 35, 40, 1, 20, 30, 1, 15, 20, 1, 0, 5, 1, 2.0, 18.9, 1, 0.3, 10);
    query1(C, 0, 35, 40, 0, 20, 30, 0, 15, 20, 0, 0, 5, 1, 2.0, 18.9, 1, 0.3, 10);
    query1(C, 1, 35, 40, 1, 20, 30, 1, 15, 20, 1, 0, 5, 0, 2.0, 18.9, 0, 0.3, 10);
    query1(C, 0, 35, 40, 0, 20, 30, 0, 15, 20, 0, 0, 5, 0, 2.0, 18.9, 0, 0.3, 10);
    query2(C, "LightBlue");
    query2(C, "Gold");
    query2(C, "Maroon");
    query2(C, "DarkBlue");
    query2(C, "Red");
    query2(C, "Black");

    query3(C, "Duke");
    query3(C, "UNC");
    query3(C, "WakeForest");
    query3(C, "BostonCollege");
    query3(C, "FloridaState");
    query3(C, "Miami");

    query4(C, "NC", "LightBlue");
    query4(C, "VA", "DarkBlue");
    query4(C, "FL", "Maroon");
    query4(C, "KY", "Black");
    query4(C, "MA", "Red");
    query4(C, "GA", "Gold");

    query5(C, 5);
    query5(C, 10);
    query5(C, 15);
    query5(C, 13);
    query5(C, 12);
    query5(C, 8);
}
