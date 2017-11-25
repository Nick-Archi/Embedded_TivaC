/*
 * RxCommands.h
 *
 *  Created on: Nov 19, 2017
 *      Author: nicho
 */

//------------------------------------------
// Variables
//------------------------------------------




//------------------------------------------
// Functions
//------------------------------------------
void ReadFrame(char* localCmdBuffer);
void ReadFrame_i(char* localCmdBuffer);
void StoreReceivedCommand_W(char* localCmdBuffer);
