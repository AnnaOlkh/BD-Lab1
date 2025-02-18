#pragma once
#ifndef GARBAGE_H
#define GARBAGE_H

#include <vector>
#include <fstream>
#include <queue>

void load_trash(const char* filename, std :: queue<int>& free_ids);
void save_trash(const char* filename, std :: queue<int>& free_ids);

#endif
