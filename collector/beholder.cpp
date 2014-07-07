/* This thread will gather statistics and maybe store
 * the checkpoints for restore.
 */
#include <iostream>
#include <fstream>
#include <time.h>
#include "collector.h"
#include "beholder.h"
#include <string>
#include "DataTypes.h"

using namespace std;

// Writes data to a file
void writeStats(string elapsedTime, string pages, string totalPages){
   ofstream fout("statistics.txt", fstream::app);
   fout << elapsedTime << "\t" <<
           pages << "\t" <<
           totalPages << endl;
   fout.close();
}

void *beholder::gatherStatistics(void* noUse){
   unsigned int elapsedTime = 0;
   // # Pages seen since last reading
   unsigned long int pagesSeen = 0;
   // Total pages seen
   unsigned long int totalPages = 0;
   // Write the first values
   writeStats("0","0","0");
   int chk = 1;
   while (1){
      // Sleep during the delay time between readings
      sleep(STATISTICS_DELAY);
      // Get the new values
      totalPages = collector::viewDocID();
      pagesSeen = totalPages - pagesSeen;
      elapsedTime += STATISTICS_DELAY;
      writeStats(itos(elapsedTime/60),
                 itos(pagesSeen),
                 itos(totalPages));
      pagesSeen = totalPages;
      // Save state for possible resume
      if ((chk % CHECKPOINT_DELAY) == 0){
         collector::saveState();
      }
      chk++;
   }
}
