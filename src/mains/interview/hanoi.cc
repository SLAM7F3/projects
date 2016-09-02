// =====================================================================
// Recursive towers of Hanoi problem
// =====================================================================
// Last updated on 2/10/16
// =====================================================================

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

class hanoi
{
  public:

/*
   enum Peg
   {
      A, B, C
   };
*/

// Constructor starts with all disks on peg A:

   hanoi(int n)
   {
      n_disks = n;
      move = 0;
      
      deque<int> Adisks, Bdisks, Cdisks;
      for(unsigned int i = 0; i < n_disks; i++)
      {
         Adisks.push_back(i);
      }
      pegDisks.push_back(Adisks);
      pegDisks.push_back(Bdisks);
      pegDisks.push_back(Cdisks);

      printState();
   }

// -------------------------------------------------------------------
   void printState()
   {
      cout << "--------------------------------------" << endl;
      cout << "Towers' state after move " << move << " : " << endl;
      
      for(unsigned int p = 0; p < pegDisks.size(); p++)
      {
         switch (p)
         {
            case 0:
               cout << "A peg:" << endl;
               break;
            case 1:
               cout << "B peg:" << endl;
               break;
            case 2:
               cout << "C peg:" << endl;
               break;
         }

         for(unsigned int j = 0; j < pegDisks[p].size(); j++)
         {
            cout << pegDisks[p].at(j) << endl;
         }
         cout << endl;

      } // loop over index p labeling pegs
   }
   
// -------------------------------------------------------------------
// Boolean method moveDisk() returns false if requested move is
// illegal - i.e. value of fromPeg disk > value of toPeg disk.  

   bool moveDisk(int fromPeg, int toPeg)
   {
      int currDisk = pegDisks[fromPeg].front();
      if(pegDisks[toPeg].size() == 0 ||
         pegDisks[toPeg].front() > currDisk)
      {
         pegDisks[fromPeg].pop_front();
         pegDisks[toPeg].push_front(currDisk);
         
         move++;
         printState();
         return true;
      }
      else
      {
         return false;
      }
   }

// -------------------------------------------------------------------
   int getSparePeg(int fromPeg, int toPeg)
   {
      if( (fromPeg == 0 && toPeg == 1) ||
          (fromPeg == 1 && toPeg == 0) )
      {
         return 2;
      }
      else if( (fromPeg == 1 && toPeg == 2) ||
               (fromPeg == 2 && toPeg == 1) )
      {
         return 0;
      }
      else// if( (fromPeg == 2 && toPeg == 0) ||
//               (fromPeg == 0 && toPeg == 2) )
      {
         return 1;
      }
   }
   
// -------------------------------------------------------------------
   bool isSolved(int toPeg)
   {
      bool solved_flag = true;
      for(int p = 0; p < 3; p++)
      {
         if(p == toPeg)
         {
            if(pegDisks[p].size() != n_disks)
            {
               solved_flag = false;
               break;
            }
         }
         else
         {
            if(pegDisks[p].size() > 0)
            {
               solved_flag = false;
               break;
            }
         }
         
      }
      return solved_flag;
   }

// -------------------------------------------------------------------
   void solveProblem(int numDisksToMove, int fromPeg, int toPeg)
   {
      if(numDisksToMove == 0)
      {
         return;
      }
      else
      {
         int sparePeg = getSparePeg(fromPeg, toPeg);
         solveProblem(numDisksToMove - 1, fromPeg, sparePeg);
         moveDisk(fromPeg, toPeg);
         solveProblem(numDisksToMove - 1, sparePeg, toPeg);
      }
   }
   
// -------------------------------------------------------------------

  private:

   unsigned int n_disks;
   int move;

// Store IDs for disks currently located on pegs A, B & C within 3
// member deques:

   vector< deque<int> > pegDisks;
};

// =====================================================================

int main()
{
   int n_disks = 2;
   cout << "Enter n_disks:" << endl;
   cin >> n_disks;

   hanoi H(n_disks);

   int startPeg = 0;
   int stopPeg = 1;
   H.solveProblem(n_disks, startPeg, stopPeg);

   bool solved_flag = H.isSolved(stopPeg);
   if(solved_flag)
   {
      cout << "Problem is solved" << endl;
   }
   else
   {
      cout << "Problem is not solved" << endl;
   }
}


