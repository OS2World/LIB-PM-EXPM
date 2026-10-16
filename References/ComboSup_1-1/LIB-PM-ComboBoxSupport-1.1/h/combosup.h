#define ICB_NOSELECTION -1
USHORT EXPENTRY InitComboBox( HWND hwnd, SHORT idCombo, PSZ *apszInit,
    SHORT InitialSelection );


USHORT  EXPENTRY InitComboBoxFromRcStrings( HAB hab,
    HWND hwnd, SHORT idCombo,
    SHORT cStrings, SHORT aidStrings[], SHORT cbStringMax,
    HMODULE hmodResource, SHORT InitialSelection );
