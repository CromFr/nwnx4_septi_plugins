///////////////////////////////////////////////////////////////////////////////////////////
// sept_anticheat_levelup - Script used for demo purpose only. Write your own to manage
//										the error case.
// Original Scripter:  Septirage
//////////////////////////////////////////////////////////////////////////////////////////


/*
 * This function is called on error detected when a player try to levelup a character.
 * 
 * sPlayerName      -   The "Bioware" Account of the Player
 * oCharacter       -   The character concerned by the levelup (nothing has been applied yet)
 * sErrorList       -   The list of all errors detected during this creation
 *
 * Return value :  FALSE if this is a real error and you wish to prevent its creation. TRUE otherwise
 */
int StartingConditional(string sPlayerName, object oCharacter, string sErrorList)
{
    //here I can log sErrorList with sPlayerName.
    //I can analyse it (if i have a really specific module that cause false positive error)
    //I can also choose to do something to this nasty player !

    //For now.. It's only demo, i will do nothing and just keep it as error 
    //(yeah, for that, the best would have been not to configure the script)

    return FALSE;
}
