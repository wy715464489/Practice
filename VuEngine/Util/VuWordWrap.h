//
//
// File: wordwrap.h
//
//

#ifndef __WORDWRAP_H
#define __WORDWRAP_H

typedef VUUINT (*CB_GetWidthW)( wchar_t, const void *pUserData );

void 	WordWrap_SetOption( VUUINT uOption );
VUUINT  WordWrap_GetOption();

const wchar_t *WordWrap_FindNextLineW( const wchar_t *pszSource, VUUINT uWidth, CB_GetWidthW pGetWidthW, const void *pUserData, const wchar_t **ppszEOL );

#define WordWrap_FindNextLine		WordWrap_FindNextLineW
#define WordWrap_Tokenize			WordWrap_TokenizeW
#define WordWrap_ClearTokens		WordWrap_ClearTokensW
#define WordWrap_InitTokens		WordWrap_InitTokensW

#define	WW_PROHIBITION	0x00000001
#define	WW_NOMBCSSP		0x00000002 // specify when disable MBCS SP Process
#define	WW_NOHANGULWRAP 0x00000004 // disable Hangul Character's WordWrap

//
// for compatibillity
//
#define LB_KINSOKU					WW_PROHIBITION
#define	LB_NOMBCSSP					WW_NOMBCSSP

#define LineBreak_SetOption			WordWrap_SetOption
#define LineBreak_SetCallback		WordWrap_SetCallback

#define LineBreak_FindNextLineW		WordWrap_FindNextLineW
#define LineBreak_FindNextLineA		WordWrap_FindNextLineA

#define LineBreak_FindNextLine 	WordWrap_FindNextLineW
#define LineBreak_CharNext(cp,p) 	CharNext(p)
#define LineBreak_Tokenize			WordWrap_TokenizeW
#define LineBreak_ClearTokens		WordWrap_ClearTokensW
#define LineBreak_InitTokens		WordWrap_InitTokensW

bool WordWrap_CanBreakLineAt( const wchar_t *psz, const wchar_t *pszStart );
const wchar_t *WordWrap_FindNonWhiteSpaceForward( const wchar_t *psz );
#define WordWrap_IsWhiteSpace(c) ( ( c ) == L'\t' || ( c ) == L'\r' || ( c ) == L' ' || ( c ) == 0x3000 ) 
#define WordWrap_IsLineFeed(c) ( ( c ) == L'\n' )
#endif //__WORDWRAP_H
