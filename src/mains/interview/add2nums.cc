// =====================================================================
// Add 2 numbers
// =====================================================================
// Last updated on 1/17/17
// =====================================================================

// https://leetcode.com/problems/add-two-numbers/

// You are given two non-empty linked lists representing two
// non-negative integers.  The digits are stored in reverse order and
// each of their nodes contain a single digit. Add the two numbers and
// return it as a linked list.

// You may assume the two numbers do not contain any leading zero,
// except the number 0 itself.

// Input: (2 -> 4 -> 3) + (5 -> 6 -> 4)
// Output: 7 -> 0 -> 8

// Defintion for singly-linked list:

// class ListNode(object)
//    def __init__(self, x)
//        self.val = x
//        self.next = None

// class Solution(object):
//   def addTwoNumbers(self, l1, l2):
//   type l1: ListNode
//   type l2: ListNode
//   rtype: ListNode

#include <algorithm>
#include <deque>
#include <iostream>
#include <string>
#include <vector>

using std::cin;
using std::cout;
using std::deque;
using std::endl;
using std::string;
using std::vector;

int main()
{
   int n_disks = 2;
   cout << "Enter n_disks:" << endl;
   cin >> n_disks;
}


