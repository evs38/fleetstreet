UNIT Shotgun;


{=======================================================================================================}
{                                                                                                       }
{ Shotgun.Pas For Borland/Turbo Pascal v7.0                                                             }
{                                                                                                       }
{      Copyright (C) 1994-1996 GUNN Data Systems. All Rights Reserved.                                  }
{                                                                                                       }
{  This unit is provided as-is, without any guarantees of any kind. The use                             }
{  of this unit is NOT a form of ownership of this interface unit, nor any                              }
{      information that can be attained from it. This unit and protocol                                 }
{    specification contained within may only be used for door development.                              }
{  Any other use is strictly prohibited, and constitutes an infringement of                             }
{          copyright which is prosecutable under international laws.                                    }
{                                                                                                       }
{     If you are writing a BBS program similar to Shotgun Professional,                                 }
{       do your own work. There is no glory in stealing from others.                                    }
{     You have been given this unit because we feel you can be trusted.                                 }
{                                                                                                       }
{      THIS UNIT AND ANY INFORMATION RETRIEVED FROM THIS UNIT MAY NOT                                   }
{    BE GIVEN OUT TO OTHERS WITHOUT THE PERMISSION OF GUNN DATA SYSTEMS.                                }
{=======================================================================================================}


INTERFACE


TYPE AddrType = RECORD
     Zone     : Word;
     Net      : Word;
     Node     : Word;
     Point    : Word;
     END;


{=======================================================================================================}
{ Note: Anything listed as 'n/a' for a description simply implies your apps                             }
{       are NOT to use the variable for any purpose. It may not be stored                               }
{       with any value other than the original that was placed in the file.                             }
{       These variables are for future use.                                                             }
{=======================================================================================================}
{=======================================================================================================}
{ \SHOTGUN\FREQ\xxxxxxxx.BBS                                                                            }
{                                                                                                       }
{  - Individual file requests by the users.                                                             }
{  - xxxxxxxx corresponds to the user number in the user base.                                          }
{  - Multiple record file.                                                                              }
{                                                                                                       }
{=======================================================================================================}
TYPE _RequestFile            = RECORD
     RequestDate             : Longint;                {Date of request - packed DOS date format.       }
     FileName                : String[60];             {Full path and filename of file requested.       }
     FileArea                : Longint;
     FileGroup               : Longint;
     CDRom                   : Longint;
     END;
{=======================================================================================================}
{ \SHOTGUN\DATA\LINKS.DAT                                                                               }
{                                                                                                       }
{  - Defines a connected uplink or downlink that you receive/send mail or files.                        }
{  - Multiple record file.                                                                              }
{                                                                                                       }
{=======================================================================================================}
TYPE _Links_Record           = RECORD
     NetworkAddress          : AddrType;               {The network aka of this link.                   }
     SysopName               : String[30];             {The name of the sysop of the system.            }
     SGFileTIC               : String[8];              {Password for TIC files.                         }
     SGFileAreaFix           : String[8];              {Password for SGFile's Areafix.                  }
     SGEchoPacket            : String[8];              {Password for message bundles.                   }
     SGEchoAreaFix           : String[8];              {Password for SGEcho's Areafix.                  }
     FileGroup1              : Word;                   {Accessable file sub group number (1).           }
     FileGroup2              : Word;                   {Accessable file sub group number (2).           }
     FileGroup3              : Word;                   {Accessable file sub group number (3).           }
     FileGroup4              : Word;                   {Accessable file sub group number (4).           }
     FileGroup5              : Word;                   {Accessable file sub group number (5).           }
     FileGroup6              : Word;                   {Accessable file sub group number (6).           }
     FileGroup7              : Word;                   {Accessable file sub group number (7).           }
     FileGroup8              : Word;                   {Accessable file sub group number (8).           }
     FileGroup9              : Word;                   {Accessable file sub group number (9).           }
     FileGroup10             : Word;                   {Accessable file sub group number (10).          }
     MessageGroup1           : Word;                   {Accessable message sub group number (1).        }
     MessageGroup2           : Word;                   {Accessable message sub group number (2).        }
     MessageGroup3           : Word;                   {Accessable message sub group number (3).        }
     MessageGroup4           : Word;                   {Accessable message sub group number (4).        }
     MessageGroup5           : Word;                   {Accessable message sub group number (5).        }
     MessageGroup6           : Word;                   {Accessable message sub group number (6).        }
     MessageGroup7           : Word;                   {Accessable message sub group number (7).        }
     MessageGroup8           : Word;                   {Accessable message sub group number (8).        }
     MessageGroup9           : Word;                   {Accessable message sub group number (9).        }
     MessageGroup10          : Word;                   {Accessable message sub group number (10).       }
     AkaToUse                : Byte;                   {Index to the AKA your system is to use with     }
                                                       {this link.                                      }
     NetMailStatus           : Byte;                   {Status to be placed on netmail messages:        }
                                                       {1-None.                                         }
                                                       {2-Hold.                                         }
                                                       {3-Crash.                                        }
     AllowPause              : Boolean;                {'TRUE' to allow link to temporarily suspend     }
                                                       {message/file packing.                           }
     AllowResume             : Boolean;                {'TRUE' to allow link to resume message/file     }
                                                       {packing.                                        }
     AllowRescan             : Boolean;                {'TRUE' to allow link to rescan mail areas.      }
     AllowHelp               : Boolean;                {'TRUE' to allow link to get help (%HELP).       }
     AllowPassword           : Boolean;                {'TRUE' to allow link to change password.        }
     AllowCompress           : Boolean;                {'TRUE' to allow link to change archiver method. }
     AllowList               : Boolean;                {'TRUE' to allow link to list available areas.   }
     AllowQuery              : Boolean;                {'TRUE' to allow link to list connected areas.   }
     AllowUnlinked           : Boolean;                {'TRUE' to allow link to list areas that are     }
                                                       {available but not yet connected.                }
     AreaCreate              : Boolean;                {'TRUE' to allow link to automatically create    }
                                                       {new file and message echos on your system.      }
     Paused                  : Boolean;                {'TRUE' if the node temporarily has all message  }
                                                       {and file packing suspended.                     }
     Archiver                : String[3];              {Archiver to use for SGEcho.                     }
     END;
{=======================================================================================================}
{ \SHOTGUN\DATA\ERRLEVEL.xxx                                                                            }
{                                                                                                       }
{  - Definitions for errorlevel drops in the WFC and SGMail.                                            }
{  - File extension corresponds with the individual node.                                               }
{  - Multiple record file.                                                                              }
{                                                                                                       }
{=======================================================================================================}
TYPE _ErrLevel_Record   = RECORD
     ConnectMessage     : String[16];                  {Connect msg to look for. ie: 'CONNECT 14400'.   }
     ConnectSpeed       : Longint;                     {The actual connection speed. ie: '14400'.       }
     ErrorLevel         : Byte;                        {Errorlevel to exit to the batch file (humans).  }
     Immediate          : Boolean;                     {If 'TRUE' SGMail won't attempt a mail session,  }
                                                       {but rather just exit right to the batch file.   }
     AllowHumans        : Boolean;                     {Allow human callers at this baud rate?          }
     END;
{=======================================================================================================}
{ \SHOTGUN\DATA\EVENTSP.xxx                                                                             }
{                                                                                                       }
{  - Definitions for system polling events for SGMail.                                                  }
{  - File extension corresponds with the individual node.                                               }
{  - Multiple record file.                                                                              }
{                                                                                                       }
{=======================================================================================================}
TYPE _Poll_Event             = RECORD
     StartTime               : Longint;                {Number of seconds past midnight event starts.   }
     EndTime                 : Longint;                {Number of seconds past midnight event ends.     }
     Days                    : Array[1..7] Of Boolean; {Sunday-Saturday. 'TRUE' for active that day.    }
     Active                  : Boolean;                {'TRUE' if event is enabled.                     }
     CallAddress             : AddrType;               {Net address to dial. ie: '100:1000/0'.          }
     Description             : String[40];             {Verbal description of the event.                }
     AkaToUse                : Word;                   {n/a                                             }
     LastMonth               : Word;                   {Last numeric month event was run.               }
     LastDay                 : Word;                   {Last numeric day event was run.                 }
     LastYear                : Word;                   {Last numeric year event was run.                }
     END;
{=======================================================================================================}
{ \SHOTGUN\DATA\EVENTSS.xxx                                                                             }
{                                                                                                       }
{  - Definitions for mail send (unhold) events for SGMail.                                              }
{  - File extension corresponds with the individual node.                                               }
{  - Multiple record file.                                                                              }
{                                                                                                       }
{=======================================================================================================}
TYPE _Send_Event             = RECORD
     StartTime               : Longint;                {Number of seconds past midnight event starts.   }
     EndTime                 : Longint;                {Number of seconds past midnight event ends.     }
     Days                    : Array[1..7] Of Boolean; {Sunday-Saturday. 'TRUE' for active that day.    }
     Active                  : Boolean;                {'TRUE' if event is enabled.                     }
     CallAddress             : AddrType;               {Net address to dial. ie: '100:1000/0'.          }
     Description             : String[40];             {Verbal description of the event.                }
     AkaToUse                : Word;                   {n/a                                             }
     LastMonth               : Word;                   {Last numeric month event was run.               }
     LastDay                 : Word;                   {Last numeric day event was run.                 }
     LastYear                : Word;                   {Last numeric year event was run.                }
     END;
{=======================================================================================================}
{ \SHOTGUN\DATA\NETWORK.DAT                                                                             }
{                                                                                                       }
{  - Defines system network addresses and domain names.                                                 }
{  - Multiple record file.                                                                              }
{                                                                                                       }
{=======================================================================================================}
TYPE _Network_Record    = RECORD
     Net_Address        : AddrType;                    {Network address - first should be primary.      }
     Net_Domain         : String[15];                  {Network domain name.                            }
     END;
{=======================================================================================================}
{ \SHOTGUN\DATA\ORIGINS.DAT                                                                             }
{                                                                                                       }
{  - Defines system origin lines.                                                                       }
{  - Multiple record file.                                                                              }
{                                                                                                       }
{=======================================================================================================}
TYPE _Origin_Record     = RECORD
     Net_Origin         : String[50];                  {Origin lines for email (in step with addresses).}
     END;
{=======================================================================================================}
{ \SHOTGUN\SGMAIL\NODELIST\NODEINDX.SG                                                                  }
{                                                                                                       }
{  - AKA index for an entry in the NODELIST.SG file.                                                    }
{  - Multiple record file.                                                                              }
{                                                                                                       }
{=======================================================================================================}
TYPE _NODEINDX_SG            = RECORD
     Index                   : Word;                   {Pointer to position (zero based) in NODELIST.SG.}
     Aka                     : AddrType;               {Node's network address.                         }
     END;
{=======================================================================================================}
{ \SHOTGUN\SGMAIL\NODELIST\NODELIST.SG                                                                  }
{                                                                                                       }
{  - Nodelist entry for a single node.                                                                  }
{  - Multiple record file.                                                                              }
{                                                                                                       }
{=======================================================================================================}
TYPE _NODELIST_SG            = RECORD
     Sysop                   : String[25];             {Sysop's name.                                   }
     System                  : String[30];             {The name of the system.                         }
     Phone                   : String[40];             {The system's phone number.                      }
     Aka                     : AddrType;               {Node's network address.                         }
     Baud                    : Word;                   {Baud rate as specified in the raw nodelist.     }
     CM                      : Boolean;                {True it the system accepts 24 hour mail.        }
     END;
{=======================================================================================================}
{ \SHOTGUN\DATA\PASSWORD.DAT                                                                            }
{                                                                                                       }
{  - Structure that holds session passwords used by SGMail.                                             }
{  - Multiple record file.                                                                              }
{                                                                                                       }
{=======================================================================================================}
TYPE _SessionPW_Record  = RECORD
     SystemAddress      : AddrType;                    {The remote system's network address.            }
     SystemPW           : String[8];                   {Password to use for mail sessions.              }
     DoEMSI             : Boolean;                     {Allow EMSI sessions.                            }
     DoWaZOO            : Boolean;                     {Allow WaZOO sessions.                           }
     DoZedZap           : Boolean;                     {Allow ZedZap transfers.                         }
     PresentAllAkas     : Boolean;                     {True to send all akas to the remote.            }
     END;
{=======================================================================================================}
{ \SHOTGUN\DATA\PBOOK.DAT                                                                               }
{                                                                                                       }
{  - Phone book configuration for fast alias name lookups for SGMail.                                   }
{  - Multiple record file.                                                                              }
{                                                                                                       }
{=======================================================================================================}
TYPE _PhoneBook_Record       = RECORD
     UserName                : String[30];             {The alias name to use for the node.             }
     NetAddress              : AddrType;               {The remote system's network address.            }
     Description             : String[40];             {Brief description of this person/system.        }
     END;
{=======================================================================================================}
{ \SHOTGUN\DATA\ROUTE.DAT                                                                               }
{                                                                                                       }
{  - File that holds call information for routing netmail messages in SGMail.                           }
{  - Multiple record file.                                                                              }
{                                                                                                       }
{=======================================================================================================}
TYPE _CallRouting_Record     = RECORD
     SourceAddr              : String[20];             {The address of the recipient of the netmail.    }
                                                       {May contain wildcards, thus its a string type.  }
     DestAddr                : AddrType;               {The address of the system the netmail is routed }
                                                       {through.                                        }
     END;
{=======================================================================================================}
{ \SHOTGUN\DATA\SGECHO.DAT                                                                              }
{                                                                                                       }
{  - Holds areas that are enabled for certain systems for SGEcho.                                       }
{  - Multiple record file.                                                                              }
{                                                                                                       }
{=======================================================================================================}
TYPE _SGEchoDat_Record       = RECORD
     Aka                     : AddrType;               {Network address of the link.                    }
     AreaID                  : Word;                   {AreaID of the file echo connected to the system.}
     END;
{=======================================================================================================}
{ \SHOTGUN\DATA\SGFILE.DAT                                                                              }
{                                                                                                       }
{  - Holds areas that are enabled for certain systems for SGFile.                                       }
{  - Multiple record file.                                                                              }
{                                                                                                       }
{=======================================================================================================}
TYPE _SGFileDat_Record       = RECORD
     Aka                     : AddrType;               {Network address of the link.                    }
     AreaID                  : Word;                   {AreaID of the file echo connected to the system.}
     END;
{=======================================================================================================}
{ \SHOTGUN\DATA\UUCP.DAT                                                                                }
{                                                                                                       }
{  - Configuration for gated Fido<>UUCP messages.                                                       }
{  - Single record file.                                                                                }
{                                                                                                       }
{=======================================================================================================}
TYPE _UUCP_Record            = RECORD
     HostName                : String[40];             {The name of the host mail gate (usually UUCP).  }
     HostAddress             : AddrType;               {The network address of the gate.                }
     YourAddress             : AddrType;               {Your network address to use for outgoing mail.  }
     END;
{=======================================================================================================}
{ \SHOTGUN\DATA\INITS.xxx                                                                               }
{                                                                                                       }
{  - Definitions for pre-dial init strings for SGMail.                                                  }
{  - File extension corresponds with the individual node.                                               }
{  - Multiple record file.                                                                              }
{                                                                                                       }
{=======================================================================================================}
TYPE _Mailer_Inits           = RECORD
     InitString              : String[40];             {Initialization string.                          }
     CallAddress             : AddrType;               {Calling net address. ie: '100:1000/0'.          }
     END;
{=======================================================================================================}
{ \SHOTGUN\DATA\ACCESS.DAT                                                                              }
{                                                                                                       }
{  - Holds the system access level definitions.                                                         }
{  - Multiple record file.                                                                              }
{                                                                                                       }
{=======================================================================================================}
TYPE _Access_Record          = RECORD
     Access_Level            : Word;                   {Numeric access level.                           }
     Name                    : String[15];             {Verbal name for access level.                   }
     Time                    : Longint;                {Daily time limit in seconds.                    }
     Ratio_Start             : Longint;                {Number of DL bytes before ratio enforcement.    }
     First_Menu              : String[12];             {The users "MAIN" menu with *.MNU extension.     }
     File_Ratio              : Longint;                {Number of DL files the user gets for 1 UL file. }
     Byte_Ratio              : Longint;                {Number of DL bytes the user gets for 1 UL byte. }
     Daily_Files             : Longint;                {n/a                                             }
     Daily_Bytes             : Longint;                {Maximum number of DL bytes per day.             }
     Ul_Credit               : Byte;                   {% time return for ULed files. ie: '50'%.        }
     Page                    : Byte;                   {0-Can't page sysop, 1-Can page sysop.           }
     Tags                    : Array[1..10] Of Byte;   {Default user tags.                              }
     TBank                   : Longint;                {Time bank bonus - in seconds.                   }
     BBank                   : Longint;                {Byte bank bonus - in bytes.                     }
     Verified                : Byte;                   {0-Not verified, 1-Verified.                     }
     QWKBytes                : Longint;                {Maximum bytes per QWK packet.                   }
     QWKMessages             : Word;                   {Maximum messages per QWK packet.                }
     QWKFree                 : Byte;                   {0-Normal, 1-Time returned for QWK downloads.    }
     NoFileRatio             : Byte;                   {0-Normal, 1-No file ratio enforcing.            }
     SubModel                : Byte;                   {Subscription model:                             }
                                                       {0-None                                          }
                                                       {1-By calendar days                              }
                                                       {2-By call days                                  }
                                                       {3-By minutes used                               }
                                                       {4-Specific expiry date                          }
     SubMinutes              : Longint;                {Number of max minutes for subscription.         }
     SubCalendarDays         : Longint;                {Number of max calendar days for subscription.   }
     SubCallDays             : Longint;                {Number of max call days for subscription.       }
     ExpiryAccessLevel       : Word;                   {Access level to expire to.                      }
     END;
{=======================================================================================================}
{ \SHOTGUN\DATA\ARCHIVE.DAT                                                                             }
{                                                                                                       }
{  - Holds global definitions for archiver utilities.                                                   }
{  - Paths MUST contain a trailing backslash.                                                           }
{  - Single record file.                                                                                }
{                                                                                                       }
{=======================================================================================================}
TYPE _Archiver_Record        = RECORD
     DefaultArc              : String[3];              {Default archive extension. ie: 'ZIP'.           }
     ArcPath                 : String[40];             {Full path to location of archiver utilities.    }
     ConvertArcs             : Byte;                   {0-Normal, 1-Convert archives to system default. }
     END;
{=======================================================================================================}
{ \SHOTGUN\DATA\ARCUTILS.DAT                                                                            }
{                                                                                                       }
{  - Holds def itions for individual archivers used on the system.                                     }
{  - Paths MUST contain a trailing backslash.                                                           }
{  - Multiple record file.                                                                              }
{                                                                                                       }
{=======================================================================================================}
TYPE _Archiver_Utilities     = RECORD
     Extension               : String[3];              {File extension of archives used with this util. }
     CopyArc                 : String[40];             {Full file name/command for adding files to arc. }
     MoveArc                 : String[40];             {Full file name/command for moving files to arc. }
     ExtractArc              : String[40];             {Full file name/command for extracting from arc. }
     ViewArc                 : String[40];             {Full file name/command for creating arc list.   }
     END;
{=======================================================================================================}
{ \SHOTGUN\DATA\BADLANG.DAT                                                                             }
{                                                                                                       }
{  - Holds definitions for words/phrases to be removed from mail areas.                                 }
{  - Strings MUST by in upper case characters.                                                          }
{  - Multiple record file.                                                                              }
{                                                                                                       }
{=======================================================================================================}
TYPE _BadLang_Record         = RECORD
     Phrase                  : String[30];             {Word/phrase to have censored.                   }
     END;
{=======================================================================================================}
{ \SHOTGUN\DATA\CALLBACK.DAT                                                                            }
{                                                                                                       }
{  - Holds general configuration for the callback verifier.                                             }
{  - Single record file.                                                                                }
{                                                                                                       }
{=======================================================================================================}
TYPE _CallBack_Record        = RECORD
     ForceUse                : Byte;                   {0-Bypass the CBV, 1-Use always.                 }
     VerifyLD                : Byte;                   {0-Normal, 1-Allow verification of LD callers.   }
     VAccess                 : Word;                   {Numeric access level after verification.        }
     CBVStart                : Longint;                {Seconds past midnight CBV can begin.            }
     CBVEnd                  : Longint;                {Seconds past midnight CBV will halt.            }
     Template                : String[20];             {n/a                                             }
     LDPass                  : Byte;                   {0-Normal, 1-Pass long distance callers through. }
     UserToLogBackIn         : Byte;                   {0-Normal, 1-User must call back after verifying.}
     CallRetries             : Byte;                   {Number of calls to be made before giving up.    }
     Scrap                   : Array[1..126] Of Byte;  {n/a                                             }
     END;
{=======================================================================================================}
{ SHOTGUN\DATA\TRANSLAT.DAT                                                                            }
{                                                                                                       }
{  - Holds dial translations used for the callback verifier.                                            }
{  - Multiple record file.                                                                              }
{                                                                                                       }
{=======================================================================================================}
TYPE _CallBack_Translation   = RECORD
     LookFor                 : String[12];             {Search string.                                  }
     Replacement             : String[12];             {Replacement string.                             }
     END;
{=======================================================================================================}
{ \SHOTGUN\DATA\CDROMS.DAT                                                                              }
{                                                                                                       }
{  - Holds definitions for each CD Rom used on the system.                                              }
{  - Multiple record file.                                                                              }
{                                                                                                       }
{=======================================================================================================}
TYPE _CDRom_Record           = RECORD
     CDName                  : String[30];             {Descriptive name of the CD Rom.                 }
     CDEnabled               : Byte;                   {0-CD is offline, 1-CD is online.                }
     CDNumber                : Word;                   {The numerical reference used by the system.     }
     END;
{=======================================================================================================}
{ \SHOTGUN\ENGLISH\LANG\COLOURS.ANS                                                                     }
{                                                                                                       }
{  - Holds the colour setup for the text mode interfaces.                                               }
{  - Single record file.                                                                                }
{                                                                                                       }
{=======================================================================================================}
TYPE _SysColourANSI          = RECORD
     InputFieldForeground    : Byte;                   {Data entry text.                                }
     InputFieldBackGround    : Byte;                   {Data entry background text.                     }
     OptionBracket           : Byte;                   {Brackets that surround keypresses.              }
     OptionHotKey            : Byte;                   {Hotkeys used for options.                       }
     OptionText              : Byte;                   {Descriptive text of options.                    }
     ChatSysop               : Byte;                   {Text colour of sysop's text in chat.            }
     ChatUser                : Byte;                   {Text colour of user's text in chat.             }
     FileListNumber          : Byte;                   {File number in file listings.                   }
     FileListBracket         : Byte;                   {Bracket after file number in file listings.     }
     FileListFileName        : Byte;                   {File name in file listings.                     }
     FileListFileSize        : Byte;                   {File size in file listings.                     }
     FileListFileDate        : Byte;                   {File date in file listings.                     }
     FileListDescription1    : Byte;                   {First line of a file's description.             }
     FileListDescriptions    : Byte;                   {Subsequent lines of a file's description.       }
     FileListTaggedFile      : Byte;                   {File name of a tagged file.                     }
     FileListOfflineFile     : Byte;                   {File 'Offline' of an offline file.              }
     FileListSeperator       : Byte;                   {Seperator bars used in file listings.           }
     ViewQuoteMSGNumber      : Byte;                   {Line editor - line number in msg quote.         }
     ViewQuoteMSGBracket     : Byte;                   {Line editor - bracket after line number.        }
     ViewQuoteMSGText        : Byte;                   {Line editor - body text in msg quote.           }
     ReadMSGQuoteColour      : Byte;                   {Msg reader - quoted text colour.                }
     ReadMSGBodyColour       : Byte;                   {Msg reader - body text colour.                  }
     LineEditorFrame         : Byte;                   {Line editor - lines that make up the borders.   }
     LineEditorLineNumbers   : Byte;                   {Line editor - line number indicators.           }
     LineEditorInfoText      : Byte;                   {Line editor - descriptive fields. ie: 'To:'.    }
     LineEditorInfoData      : Byte;                   {Line editor - actual data. ie: 'John Doe'.      }
     LineEditorHeaderFore    : Byte;                   {Line editor - text colour in screen header.     }
     LineEditorHeaderBack    : Byte;                   {Line editor - background of screen header.      }
     SpinCursor              : Byte;                   {Colour of spin cursor. ie: '/-\|'.              }
     ReadMSGTearColour       : Byte;                   {Msg reader - colour of tear/kludge lines.       }
     Scrap                   : Array[1..510] Of Byte;  {n/a                                             }
     END;
{=======================================================================================================}
{ \SHOTGUN\ENGLISH\LANG\COLOURS.VGA                                                                     }
{                                                                                                       }
{  - Holds the colour setup for the SuperVGA interface.                                                 }
{  - Single record file.                                                                                }
{                                                                                                       }
{=======================================================================================================}
TYPE _SysColourSVGA          = RECORD
     ReadMsgBody             : Byte;                   {Msg reader - body text colour.                  }
     ReadMsgQuote            : Byte;                   {Msg reader - quoted text colour.                }
     FileListFileName        : Byte;                   {File name in file listings.                     }
     FileListFileSize        : Byte;                   {File size in file listings.                     }
     FileListFileDate        : Byte;                   {File date in file listings.                     }
     FileListDescription1    : Byte;                   {First line of a file's description.             }
     FileListDescriptions    : Byte;                   {Subsequent lines of a file's description.       }
     FileListTaggedFile      : Byte;                   {File name of a tagged file.                     }
     FileListOfflineFile     : Byte;                   {File 'Offline' of an offline file.              }
     PickListBack            : Byte;                   {Picklist/work area background.                  }
     PickListFore            : Byte;                   {Picklist/work area normal text.                 }
     PickHighlightBack       : Byte;                   {Picklist/work area highlighted background.      }
     PickHighlightFore       : Byte;                   {Picklist/work area highlighted text.            }
     InternalImage           : Byte;                   {Internal image to use for backdrop:             }
                                                       {0-None.                                         }
                                                       {1-Checker board.                                }
                                                       {2-Ocean view.                                   }
     DisabledBackground      : Byte;                   {Disabled window header background.              }
     DisabledHighlight       : Byte;                   {Disabled window header highlight colour.        }
     DisabledShadow          : Byte;                   {Disabled window header shadow colour.           }
     DisabledText            : Byte;                   {Disabled window header text colour.             }
     FileQueueName           : Byte;                   {File name in download queue.                    }
     FileQueueDesc           : Byte;                   {File description in download queue.             }
     FileQueueSize           : Byte;                   {File size in download queue.                    }
     FileQueueTime           : Byte;                   {Files' transfer time in download queue.         }
     MsgListFrom             : Byte;                   {Message listing msg 'from'.                     }
     MsgListTo               : Byte;                   {Message listing msg 'to'.                       }
     MsgListSubject          : Byte;                   {Message listing msg 'subject'.                  }
     MsgListPrivate          : Byte;                   {Message listing msg private flag.               }
     ReadMSGTearColour       : Byte;                   {Msg reader - colour of tear/kludge lines.       }
     Scrap                   : Array[1..513] Of Byte;  {n/a                                             }
     END;
{=======================================================================================================}
{ \SHOTGUN\DATA\EVENTSB.xxx                                                                             }
{                                                                                                       }
{  - Definitions for busy events for SGMail.                                                            }
{  - File extension corresponds with the individual node.                                               }
{  - Multiple record file.                                                                              }
{                                                                                                       }
{=======================================================================================================}
TYPE _Drop_Event             = RECORD
     StartTime               : Longint;                {Number of seconds past midnight event starts.   }
     EndTime                 : Longint;                {Number of seconds past midnight event ends.     }
     Days                    : Array[1..7] Of Boolean; {Sunday-Saturday. 'TRUE' for active that day.    }
     Active                  : Boolean;                {'TRUE' if event is enabled.                     }
     Description             : String[40];             {Verbal description of the event.                }
     Scrap                   : Array[1..32] Of Byte;   {n/a                                             }
     END;
{=======================================================================================================}
{ \SHOTGUN\DATA\EVENTSE.xxx                                                                             }
{                                                                                                       }
{  - Definitions for errorlevel exit events for SGMail.                                                 }
{  - File extension corresponds with the individual node.                                               }
{  - Multiple record file.                                                                              }
{                                                                                                       }
{=======================================================================================================}
TYPE _Exit_Event             = RECORD
     Time                    : Longint;                {Number of seconds past midnight event runs.     }
     ErrorLevel              : Byte;                   {DOS errorlevel to exit with.                    }
     Days                    : Array[1..7] Of Boolean; {Sunday-Saturday. 'TRUE' for active that day.    }
     Active                  : Boolean;                {'TRUE' if event is enabled.                     }
     LastMonth               : Word;                   {Last numeric month event was run.               }
     LastDay                 : Word;                   {Last numeric day event was run.                 }
     LastYear                : Word;                   {Last numeric year event was run.                }
     Description             : String[40];             {Verbal description of the event.                }
     Scrap                   : Array[1..32] Of Byte;   {n/a                                             }
     END;
{=======================================================================================================}
{ \SHOTGUN\DATA\EVENTSM.xxx                                                                             }
{                                                                                                       }
{  - Definitions for mail only events for SGMail.                                                       }
{  - File extension corresponds with the individual node.                                               }
{  - Multiple record file.                                                                              }
{                                                                                                       }
{=======================================================================================================}
TYPE _Mail_Event             = RECORD
     StartTime               : Longint;                {Number of seconds past midnight event starts.   }
     EndTime                 : Longint;                {Number of seconds past midnight event ends.     }
     Days                    : Array[1..7] Of Boolean; {Sunday-Saturday. 'TRUE' for active that day.    }
     Active                  : Boolean;                {'TRUE' if event is enabled.                     }
     Description             : String[40];             {Verbal description of the event.                }
     Scrap                   : Array[1..32] Of Byte;   {n/a                                             }
     END;
{=======================================================================================================}
{ \SHOTGUN\SGMAIL\NETMAIL\FAXxxxxx.FAX                                                                  }
{                                                                                                       }
{  - Information for a single received Fax file through SGMail.                                         }
{  - Single record file.                                                                                }
{                                                                                                       }
{=======================================================================================================}
TYPE _FaxBaseHeader_Record   = RECORD
     FaxFile                 : String[12];             {The name of the received fax file.              }
     Received                : Boolean;                {'TRUE' if fax has been read by sysop.           }
     StationID               : String[30];             {The station ID (usually phone #) of the sender. }
     Year                    : Word;                   {Numeric year the fax was received.              }
     Month                   : Word;                   {Numeric month the fax was received.             }
     Day                     : Word;                   {Numeric day the fax was received.               }
     DayOfWeek               : Word;                   {Numeric day of the week the fax was received.   }
     Hour                    : Word;                   {Numeric hour the fax was received.              }
     Minute                  : Word;                   {Numeric minute the fax was received.            }
     Second                  : Word;                   {Numeric second the fax was received.            }
     Sec100                  : Word;                   {1/100ths of a second the fax was received.      }
     END;
{=======================================================================================================}
{ xxxxxxxx.FIL                                                                                          }
{                                                                                                       }
{  - Information for a single stored file on the system.                                                }
{  - Multiple record file.                                                                              }
{                                                                                                       }
{=======================================================================================================}
TYPE _FileDefinitions        = RECORD
     File_Name               : String[12];             {File name - must be upper case.                 }
     File_Size               : Longint;                {File size in bytes.                             }
     File_Date               : Longint;                {File date - packed DOS date format.             }
     First_Desc              : Longint;                {Index to first description line in the *.TXT    }
                                                       {file - '1' based.                               }
     Desc_Lines              : Byte;                   {Number of description lines in the *.TXT file.  }
     Class                   : Byte;                   {n/a                                             }
     Free                    : Byte;                   {0-Normal, 1-No ratio/time count for DLing.      }
     DLed                    : Longint;                {Number of times the file was downloaded.        }
     Uploader                : String[30];             {Name of the user that uploaded the file.        }
     Added_Date              : Longint;                {Date the file was put online - packed DOS date. }
     Last_DLed               : Longint;                {The last date the file was downloaded - packed  }
                                                       {DOS date format.                                }
     FileStatus              : Byte;                   {n/a                                             }
     END;
{=======================================================================================================}
{ xxxxxxxx.TXT                                                                                          }
{                                                                                                       }
{  - Holds the text description for a stored file on the system.                                        }
{  - The file is arranged according to the indexes in the *.FIL files.                                  }
{  - Multiple record file.                                                                              }
{                                                                                                       }
{=======================================================================================================}
TYPE _FileDescriptions       = RECORD
     Desc                    : String[46];             {A single line of a file's description.          }
     END;
{=======================================================================================================}
{ \SHOTGUN\DATA\FAxxxxxx.DAT                                                                            }
{                                                                                                       }
{  - Definition of a single file area on the system.                                                    }
{  - Paths MUST contain a trailing backslash.                                                           }
{  - Multiple record file.                                                                              }
{                                                                                                       }
{=======================================================================================================}
TYPE _FileArea_Record        = RECORD
     Area_Name               : String[40];             {The descriptive name of the file area.          }
     Sub                     : Word;                   {The file sub group that the area resides.       }
                                                       {This must NOT be '0' and the group must exist.  }
     Dos_Name                : String[50];             {The path and file name (no extension!) of the   }
                                                       {file area's *.FIL and *.TXT database files.     }
     View_Access             : Word;                   {Minimum access level required to see the area.  }
     Dl_Access               : Word;                   {Minimum access level required to DL files.      }
     Tags                    : Array[1..10] Of Byte;   {User tags required to see the file area.        }
     Cd_Rom                  : Word;                   {If a CD Rom, the 'number' of the CD rom in the  }
                                                       {CD Roms definitions. Otherwise '0'.             }
     Sort                    : Byte;                   {Area sorting method:                            }
                                                       {1-None.                                         }
                                                       {2-Alphabetical (A-Z)                            }
                                                       {3-Alphabetical (Z-A)                            }
                                                       {4-Date (New-Old)                                }
                                                       {5-Date (Old-New)                                }
                                                       {6-Size (Large-Small)                            }
                                                       {7-Size (Small-Large)                            }
     Area_Path               : String[40];             {Path to where the files are stored.             }
     Area_Tag                : String[12];             {TIC file echo tag (if applicable).              }
     Num_Files               : Word;                   {Number of files in the area.                    }
     Num_Bytes               : Longint;                {Total bytes of files in the area.               }
     AreaID                  : Word;                   {Unique file area ID number. Every area must     }
                                                       {have an area ID different from all other areas! }
     TicConvertArcs          : Boolean;                {'TRUE' to convert archives when uploaded.       }
     TicDestFormat           : String[3];              {Target archive format extension.                }
     TicImportDiz            : Boolean;                {'TRUE' to use FILE_ID.DIZ files for description }
                                                       {purposes (if contained in the archive).         }
     TicReportNumber         : Word;                   {Report number to use for announcing uploads.    }
     Scrap                   : Array[1..123] Of Byte;  {n/a                                             }
     END;
{=======================================================================================================}
{ \SHOTGUN\DATA\FILEAREA.SUB                                                                            }
{                                                                                                       }
{  - Definition of a single file sub group on the system.                                               }
{  - Paths MUST contain a trailing backslash.                                                           }
{  - Multiple record file.                                                                              }
{                                                                                                       }
{=======================================================================================================}
TYPE _FileSub_Record         = RECORD
     Sub_Num                 : Word;                   {The numerical sub group number - for reference. }
                                                       {MUST NOT be '0'.                                }
     Sub_Name                : String[30];             {The descriptive name of the sub group.          }

        { The remainder of the record is for setting defaults for new areas created via SGConfig and    }
        { SGFile (the file echo processor).                                                             }

     View_Access             : Word;                   {Minimum access level required to see the area.  }
     DL_Access               : Word;                   {Minimum access level required to DL files.      }
     Tags                    : Array[1..10] Of Byte;   {User tags required to see the file area.        }
     Cd_Rom                  : Word;                   {If a CD Rom, the 'number' of the CD rom in the  }
                                                       {CD Roms definitions. Otherwise '0'.             }
     Sort                    : Byte;                   {Area sorting method:                            }
                                                       {1-None.                                         }
                                                       {2-Alphabetical (A-Z)                            }
                                                       {3-Alphabetical (Z-A)                            }
                                                       {4-Date (New-Old)                                }
                                                       {5-Date (Old-New)                                }
                                                       {6-Size (Large-Small)                            }
                                                       {7-Size (Small-Large)                            }
     Area_Path               : String[40];             {Path to where the files are stored.             }
     TicConvertArcs          : Boolean;                {'TRUE' to convert archives when uploaded.       }
     TicDestFormat           : String[3];              {Target archive format extension.                }
     TicImportDiz            : Boolean;                {'TRUE' to use FILE_ID.DIZ files for description }
                                                       {purposes (if contained in the archive).         }
     TicReportNumber         : Word;                   {Report number to use for announcing uploads.    }
     ScanAtLogon             : Boolean;                {'TRUE' if the area is to be scanned in the new  }
                                                       {file search at logon.                           }
     Scrap                   : Array[1..127] Of Byte;  {n/a                                             }
     END;
{=======================================================================================================}
{ \SHOTGUN\DATA\MAxxxxxx.DAT                                                                            }
{                                                                                                       }
{  - Definition of a single message area on the system.                                                 }
{  - Paths MUST contain a trailing backslash.                                                           }
{  - Multiple record file.                                                                              }
{                                                                                                       }
{=======================================================================================================}
TYPE _MessageArea_Record     = RECORD
     Scrap1                  : Word;                   {n/a                                             }
     Area_Name               : String[40];             {The descriptive name of the message area.       }
     Sub                     : Word;                   {The message sub group that the area resides.    }
                                                       {This must NOT be '0' and the group must exist.  }
     Dos_Name                : String[50];             {Path/Filename of the message base. For FIDO MSG }
                                                       {areas, this must be a directory name, and for   }
                                                       {the others, it is a path and a filename (with   }
                                                       {no extension) of the message base.              }
     Sysop                   : String[30];             {The name of the sysop of the area.              }
     Base_Type               : Byte;                   {Message base format:                            }
                                                       {1-Jam.                                          }
                                                       {2-Squish.                                       }
                                                       {3-Fido.                                         }
     Read_Access             : Word;                   {Minimum access level required to read messages. }
     Write_Access            : Word;                   {Minimum access level required to write messages.}
     Area_Type               : Byte;                   {Message type:                                   }
                                                       { Local:    1-Public  2-Priv  3-Public/Priv      }
                                                       {  Echo:   10-Public 11-Priv 12-Public/Priv      }
                                                       {   Net:   20-Public 21-Priv 22-Public/Priv      }
                                                       {  Uucp:   30-Public 31-Priv 32-Public/Priv      }
     Msg_Type                : Byte;                   {Name to use for messages:                       }
                                                       {1-Real name.                                    }
                                                       {2-Alias name.                                   }
                                                       {3-Real/Alias name.                              }
     File_Attaches           : Byte;                   {0-Normal, 1-Allow file attaches.                }
     File_Attaches_Sec       : Word;                   {Minimum access level for attaching files to     }
                                                       {messages in this area.                          }
     Tags                    : Array[1..10] Of Byte;   {User tags required to see the message area.     }
     Origin                  : Byte;                   {Index to the origin line used for this area.    }
     Address                 : Byte;                   {Index to the network address used for this area.}
     NodeList                : String[50];             {Path to the *.SG nodelist files (for netmail).  }
     DoGraphics              : Byte;                   {0-Normal, 1-Allow colour codes in message base. }
     CleanLanguage           : Byte;                   {0-Normal, 1-Censor language using BADLANG.      }
     AreaID                  : Word;                   {Unique message area ID number. Every area must  }
                                                       {have an area ID different from all other areas! }
     MaxMsgs                 : Word;                   {Maximum number of messages to keep in the area. }
     ForcedTo                : Byte;                   {0-Normal, 1-User can't change the msg recipient.}
     Area_Tag                : String[40];             {Mail echo tag (if applicable).                  }
     Scrap2                  : Array[1..84] Of Byte;   {n/a                                             }
     END;
{=======================================================================================================}
{ \SHOTGUN\DATA\MSGAREA.SUB                                                                             }
{                                                                                                       }
{  - Definition of a single message sub group on the system.                                            }
{  - Paths MUST contain a trailing backslash.                                                           }
{  - Multiple record file.                                                                              }
{                                                                                                       }
{=======================================================================================================}
TYPE _MessageSub_Record      = RECORD
     Sub_Num                 : Word;                   {The numerical sub group number - for reference. }
                                                       {MUST NOT be '0'.                                }
     Sub_Name                : String[30];             {The descriptive name of the sub group.          }

        { The remainder of the record is for setting defaults for new areas created via SGConfig and    }
        { SGEcho (the echo mail processor).                                                             }

     Dos_Name                : String[50];             {Base directory for new area creation.           }
     Base_Type               : Byte;                   {Message base format:                            }
                                                       {1-Jam.                                          }
                                                       {2-Squish.                                       }
                                                       {3-Fido.                                         }
     Read_Access             : Word;                   {Minimum access level required to read messages. }
     Write_Access            : Word;                   {Minimum access level required to write messages.}
     Area_Type               : Byte;                   {Message type:                                   }
                                                       { Local:    1-Public  2-Priv  3-Public/Priv      }
                                                       {  Echo:   10-Public 11-Priv 12-Public/Priv      }
                                                       {   Net:   20-Public 21-Priv 22-Public/Priv      }
                                                       {  Uucp:   30-Public 31-Priv 32-Public/Priv      }
     Msg_Type                : Byte;                   {Name to use for messages:                       }
                                                       {1-Real name.                                    }
                                                       {2-Alias name.                                   }
                                                       {3-Real/Alias name.                              }
     File_Attaches           : Byte;                   {0-Normal, 1-Allow file attaches.                }
     File_Attaches_Sec       : Word;                   {Minimum access level for attaching files to     }
                                                       {messages in this area.                          }
     Tags                    : Array[1..10] Of Byte;   {User tags required to see the message area.     }
     Origin                  : Byte;                   {Index to the origin line used for this area.    }
     Address                 : Byte;                   {Index to the network address used for this area.}
     DoGraphics              : Byte;                   {0-Normal, 1-Allow colour codes in message base. }
     CleanLanguage           : Byte;                   {0-Normal, 1-Censor language using BADLANG.      }
     MaxMsgs                 : Word;                   {Maximum number of messages to keep in the area. }
     ScanAtLogon             : Boolean;                {'TRUE' if the area is to be scanned in the new  }
                                                       {mail search at logon.                           }
     Scrap                   : Array[1..125] Of Byte;  {n/a                                             }
     END;
{=======================================================================================================}
{ \SHOTGUN\ICONS\xxxxxxxx.16                                                                            }
{                                                                                                       }
{  - 16x16x256 colour icon files.                                                                       }
{  - Multiple record file.                                                                              }
{                                                                                                       }
{=======================================================================================================}
TYPE _Icon16x16         = RECORD
     Matrix             : Array[1..16,1..16] Of Byte;  {16x16 byte grid of colour # in the palette.     }
     END;
{=======================================================================================================}
{ \SHOTGUN\ICONS\xxxxxxxx.30                                                                            }
{                                                                                                       }
{  - 30x30x256 colour icon files.                                                                       }
{  - Multiple record file.                                                                              }
{                                                                                                       }
{=======================================================================================================}
TYPE _Icon30x30         = RECORD
     Matrix             : Array[1..30,1..30] Of Byte;  {30x30 byte grid of colour # in the palette.     }
     END;
{=======================================================================================================}
{ \SHOTGUN\DATA\LANGUAGE.DAT                                                                            }
{                                                                                                       }
{  - Definition of an available language on the system.                                                 }
{  - Paths MUST contain a trailing backslash.                                                           }
{  - Multiple record file.                                                                              }
{                                                                                                       }
{=======================================================================================================}
TYPE _LanguageDirs_Record    = RECORD
     Name                    : String[40];             {The descriptive name of the language.           }
     Directory               : String[40];             {The full path to the _base_ directory of the    }
                                                       {the language. Other directories reside within   }
                                                       {this base directory.                            }
     END;
{=======================================================================================================}
{ \SHOTGUN\DATA\LASTCALL.DAT                                                                            }
{                                                                                                       }
{  - Information on the last 10 callers to the system.                                                  }
{  - Multiple record file. This file always contains 10 records.                                        }
{                                                                                                       }
{=======================================================================================================}
TYPE _Last10Callers_Record   = RECORD
     Name                    : String[30];             {The real name of the user.                      }
     Alias                   : String[30];             {The alias name of the user (if applicable).     }
     City                    : String[30];             {The city+prov/state of the user's origin.       }
     Baud                    : String[6];              {The connect rate of his/her last session.       }
     END;
{=======================================================================================================}
{ \SHOTGUN\DATA\LASTxxxx.DAT                                                                            }
{                                                                                                       }
{  - Information on the last caller to the system (for SGMail's use).                                   }
{  - xxxx corresponds to the individual node number.                                                    }
{  - Single record file.                                                                                }
{                                                                                                       }
{=======================================================================================================}
TYPE _Lasthuman_Record       = RECORD
     Name                    : String[30];             {The real name of the user.                      }
     TimeOn                  : String[30];             {Full time/date string of his/her logon.         }
     TimeOff                 : String[30];             {Full time/date string of his/her logoff.        }
     END;
{=======================================================================================================}
{ \SHOTGUN\DATA\LCALLSGM.xxx                                                                            }
{                                                                                                       }
{  - Information on the last mail call to/from the system (for SGMail's use).                           }
{  - xxx corresponds to the individual node number.                                                     }
{  - Single record file.                                                                                }
{                                                                                                       }
{=======================================================================================================}
TYPE _LastSGMailCalls_Record = RECORD
     ToSystem                : String[40];             {Name of the last system your system called.     }
     ToLocation              : String[40];             {Location of the last system your system called. }
     ToAddress               : String[20];             {Net AKA of the last system your system called.  }
     ToCallDate              : String[20];             {Date of the last call your system made.         }
     ToCallTime              : String[20];             {Time of the last call your system made.         }
     FrmSystem               : String[40];             {Name of the last system to call your system.    }
     FrmLocation             : String[40];             {Location of the last system to call your system.}
     FrmAddress              : String[20];             {Net AKA of the last system to call your system. }
     FrmCallDate             : String[20];             {Date of the last call to your system.           }
     FrmCallTime             : String[20];             {Time of the last call to your system.           }
     CallMonth               : Word;                   {The last month this file was updated.           }
     CallDay                 : Word;                   {The last day this file was updated.             }
     BBSCalls                : Longint;                {Number of human callers for the current day.    }
     MailCalls               : Longint;                {Number of mail calls for the current day.       }
     FaxCalls                : Longint;                {Number of fax calls for the current day.        }
     END;
{=======================================================================================================}
{ \SHOTGUN\DATA\MAILER.DAT                                                                              }
{                                                                                                       }
{  - Configuration for individual SGMail nodes. Some settings are used by SG as well.                   }
{  - Paths MUST contain a trailing backslash.                                                           }
{  - Multiple record file.                                                                              }
{                                                                                                       }
{=======================================================================================================}
TYPE _Mailer_Record          = RECORD
     HandleFaxes             : Boolean;                {'TRUE' to have SGMail accept incoming faxes.    }
     Semaphores              : String[40];             {Path to semaphore directory.                    }
     NetMail                 : String[40];             {Path to netmail area.                           }
     InBound                 : String[40];             {Path to where inbound files are stored.         }
     OutBound                : String[40];             {Path to where outgoing mail bundles are stored. }
     OutPkts                 : String[40];             {Temporary outbound used by SGMail.              }
     NodeList                : String[40];             {Path to the *.SG nodelist files (for netmail).  }
     NodeName                : String[40];             {Name of this node - usually the BBS name.       }
     Banner                  : String[40];             {Path and file name of text file that is sent    }
                                                       {to the user when the system passes to the BBS.  }
     InEvent                 : String[40];             {Path and file name of text file that is sent    }
                                                       {to the user when the system is mail only.       }
     AliasList               : String[40];             {Path and file name of text file used to define  }
                                                       {alias file names for file requests.             }
     DirList                 : String[40];             {Path and file name of text file used to define  }
                                                       {searchable paths for file requests.             }
     NoMatch                 : String[40];             {Path and file name of text file sent to remote  }
                                                       {systems when a file request fails.              }
     Phone                   : String[20];             {The phone number of the individual node.        }
     Flags                   : String[20];             {Nodelist flags for the individual node.         }
     BaudRate                : Longint;                {Port speed setting for the node. ie: 38400.     }
     AllowFreqs              : Boolean;                {'TRUE' to allow file requests.                  }
     FreqBaud                : Longint;                {Minimum connect speed to allow file requests.   }
     FreqStart               : Longint;                {Minutes past midnight file requests commence.   }
     FreqEnd                 : Longint;                {Minutes past midnight file requests end.        }
     MailOnly                : Boolean;                {'TRUE' if the individual node is mail-only.     }
     DelayFirst              : Boolean;                {'TRUE' to delay the first mail call by the      }
                                                       {number of seconds defined in RetryDelay.        }
     MatchZones              : Boolean;                {'TRUE' if the system should try and match akas  }
                                                       {with the remote system by using its zone number.}
     PresentAkas             : Boolean;                {'TRUE' if the system should present all of your }
                                                       {network addresses to the remote system.         }
     MailExit                : Byte;                   {DOS errorlevel SGMail should exit with when any }
                                                       {mail/files are received.                        }
     BusyRetries             : Byte;                   {n/a                                             }
     ResendRetries           : Byte;                   {n/a                                             }
     RetryDelay              : Byte;                   {Number of seconds between outgoing calls.       }
     AnswerOnRing            : Byte;                   {Number of rings before SGMail answers the phone.}
     BBSSoftware             : String[15];             {The name of your BBS software.                  }
     Scrap                   : Byte;                   {n/a                                             }
     SecondsToBBS            : Byte;                   {Number of seconds before the system considers   }
                                                       {inbound calls to be human callers and passes    }
                                                       {them to the BBS.                                }
     END;
{=======================================================================================================}
{ \SHOTGUN\ENGLISH\MENUS\xxxxxxxx.MNU                                                                   }
{                                                                                                       }
{  - Entry for a menu item.                                                                             }
{  - Due to the fact that a general file structure is used for many very different menu option types    }
{    (over 100), the specifics of certain fields are not given to save space.                           }
{  - Multiple record file.                                                                              }
{                                                                                                       }
{=======================================================================================================}
TYPE _Menu_Record            = RECORD
     Title                   : String[30];             {Menu option title.                              }
     HotKey                  : Char;                   {Keypress required to select the option.         }
     Option                  : Word;                   {n/a                                             }
     Second                  : String[40];             {n/a                                             }
     Third                   : String[40];             {n/a                                             }
     Access                  : Word;                   {Minimum access level required to see the option.}
     Tags                    : Array[1..10] Of Byte;   {User tags required to see the menu option.      }
     Sub                     : Word;                   {n/a                                             }
     Area                    : Word;                   {n/a                                             }
     Ansi                    : Byte;                   {1 to enable the option for the ANSI interface.  }
     Rip                     : Byte;                   {1 to enable the option for the RIP interface.   }
     Svga                    : Byte;                   {1 to enable the option for the SVGA interface.  }
     Scrap                   : Array[1..17] Of Byte;   {n/a                                             }
     END;
{=======================================================================================================}
{ \SHOTGUN\DATA\MODEMS.DAT                                                                              }
{                                                                                                       }
{  - Data file for default modem settings.                                                              }
{  - Multiple record file.                                                                              }
{                                                                                                       }
{=======================================================================================================}
TYPE _Modems_Record     = RECORD
     ModemName          : String[60];                  {The brand name/model of the modem.              }
     BaudRate           : Byte;                        {Port speed to use:                              }
                                                       {1-300.                                          }
                                                       {2-600.                                          }
                                                       {3-1200.                                         }
                                                       {4-2400.                                         }
                                                       {5-4800.                                         }
                                                       {6-9600.                                         }
                                                       {7-19200.                                        }
                                                       {8-38400.                                        }
                                                       {9-57600.                                        }
                                                       {10-115200.                                      }
     Hangup             : String[20];                  {Hayes command to hang up the modem.             }
     Offhook            : String[20];                  {Hayes command to take the modem off hook.       }
     Init1              : String[60];                  {1st initialization string.                      }
     Init2              : String[60];                  {2nd initialization string.                      }
     Init3              : String[60];                  {3rd initialization string.                      }
     Answer             : String[20];                  {Hayes command to answer the phone.              }
     CommandDelay       : Word;                        {Milliseconds between sent command strings.      }
     CompCodes          : String[20];                  {Compatability codes (protocols).                }
     DialString         : String[20];                  {Hayes command to use as a dial prefix.          }
     ConnectSpeed       : Array[1..30] Of Longint;     {Connect speed values.                           }
     ConnectString      : Array[1..30] Of String[30];  {Connect messages from the modem.                }
     END;
{=======================================================================================================}
{ \SHOTGUN\ICONS\xxxxxxxx.MSE                                                                           }
{                                                                                                       }
{  - Cursor/mask structure for mouse pointers in the SuperVGA.                                          }
{  - Multiple record file.                                                                              }
{                                                                                                       }
{=======================================================================================================}
TYPE _Mouse_Def    = RECORD
     Cursor        : Array[1..16,1..16] Of Boolean;    {Cursor bit matrix. 'TRUE' for bit set.          }
     Mask          : Array[1..16,1..16] Of Boolean;    {Cursor mask bit matrix. 'TRUE' for bit set.     }
     Colour        : Word;                             {Cursor colour index to palette.                 }
     HotX          : Word;                             {Pointer's X hotspot (from upper right corner).  }
     HotY          : Word;                             {Pointer's Y hotspot (from upper right corner).  }
     END;
{=======================================================================================================}
{ \SHOTGUN\DATA\NAMES.DAT                                                                               }
{                                                                                                       }
{  - Defines which of the user's names to use in system screens.                                        }
{  - 0-User's real name, 1-User's alias name.                                                           }
{  - Single record file.                                                                                }
{                                                                                                       }
{=======================================================================================================}
TYPE _ScreenNames_Record     = RECORD
     LastCall                : Byte;                   {Name to be used in LASTCALL.ANS/ASC/RIP/SG.     }
     LastComm                : Byte;                   {Name to be used in LASTCOMM.ANS/ASC/RIP/SG.     }
     PageUser                : Byte;                   {Name to be used in PAGEUSER.ANS/ASC/RIP/SG.     }
     UrPaged                 : Byte;                   {Name to be used in URPAGED.ANS/ASC/RIP/SG.      }
     UserList                : Byte;                   {Name to be used in USERLIST.ANS/ASC/RIP/SG.     }
     END;
{=======================================================================================================}
{ \SHOTGUN\ENGLISH\LANG\NEWUSER.ANS                                                                     }
{                                                                                                       }
{  - Defines the sequence of events of a new user logon in ANSI/ASCII/RIP mode.                         }
{  - Multiple record file.                                                                              }
{                                                                                                       }
{=======================================================================================================}
TYPE _NewUserANSI_Record     = RECORD
     FunctionType            : Byte;                   {Function to perform:                            }
                                                       {1-Display File.                                 }
                                                       {2-Get Password.                                 }
                                                       {3-Verify Password.                              }
                                                       {4-Get Data Phone Number.                        }
                                                       {5-Get Voice Phone Number.                       }
                                                       {6-Get Street Address.                           }
                                                       {7-Get City.                                     }
                                                       {8-Get Province/State.                           }
                                                       {9-Get Country.                                  }
                                                       {10-Get Postal Code.                             }
                                                       {11-Get Alias.                                   }
                                                       {12-Get Birthday.                                }
                                                       {13-Get Gender.                                  }
                                                       {14-Get IBM Characters.                          }
                                                       {15-Get BBS Name.                                }
                                                       {16-Get BBS Phone Number.                        }
                                                       {17-Get Network Address.                         }
                                                       {18-Get Enter Keypress.                          }
     Required                : Byte;                   {0-Normal, 1-User input required.                }
     FileName                : String[8];              {File name (no extension) of display files.      }
     END;
{=======================================================================================================}
{ \SHOTGUN\ENGLISH\LANG\NEWUSER.VGA                                                                     }
{                                                                                                       }
{  - Defines the sequence of events of a new user logon in SuperVGA mode.                               }
{  - Multiple record file.                                                                              }
{                                                                                                       }
{=======================================================================================================}
TYPE _NewUserVGA_Record      = RECORD
     FileName                : String[8];              {File name (no extension) of display files.      }
     END;
{=======================================================================================================}
{ \SHOTGUN\DATA\NODES.DAT                                                                               }
{                                                                                                       }
{  - Basic settings for an individual node.                                                             }
{  - Multiple record file.                                                                              }
{                                                                                                       }
{=======================================================================================================}
TYPE _Node_Record       = RECORD
     ANSIOk             : Byte;                        {1-ANSI logons ok for this node.                 }
     TTYOk              : Byte;                        {1-ASCII logons ok for this node.                }
     SVGAOk             : Byte;                        {1-SuperVGA logons ok for this node.             }
     RIPOk              : Byte;                        {1-RIP logons ok for this node.                  }
     ComDevice          : Byte;                        {Com device in use:                              }
                                                       {1-Standard UART.                                }
                                                       {2-Fossil driver.                                }
                                                       {3-DigiBOARD.                                    }
     ComPort            : Byte;                        {Com port to use.                                }
     BaudRate           : Byte;                        {Port speed to use:                              }
                                                       {1-300.                                          }
                                                       {2-600.                                          }
                                                       {3-1200.                                         }
                                                       {4-2400.                                         }
                                                       {5-4800.                                         }
                                                       {6-9600.                                         }
                                                       {7-19200.                                        }
                                                       {8-38400.                                        }
                                                       {9-57600.                                        }
                                                       {10-115200.                                      }
     LockRate           : Byte;                        {Same as baud rate.                              }
     FlowControl        : Byte;                        {Flow control type:                              }
                                                       {0-None.                                         }
                                                       {1-CTS/RTS.                                      }
                                                       {2-DTR/DSR.                                      }
                                                       {3-XON/XOFF.                                     }
     CommandDelay       : Word;                        {Milliseconds between sent command strings.      }
     InitString         : Array[1..3] Of String[40];   {Initialization strings.                         }
     OnHook             : String[16];                  {Hayes command to place the modem on hook.       }
     Offhook            : String[16];                  {Hayes command to take the modem off hook.       }
     Answer             : String[16];                  {Hayes command to answer the phone.              }
     Dial               : String[16];                  {Hayes command to use as a dial prefix.          }
     AllowNewUsers      : Byte;                        {1-Allow new users to this node.                 }
     AccessMinimum      : Word;                        {Minimum access level required to log on to this }
                                                       {node.                                           }
     Tags               : Array[1..10] Of Byte;        {User access tags required to log on to this node}
     UseBlaster         : Boolean;                     {'TRUE' to use SB for paging sysop etc.          }
     Fifos              : Byte;                        {Number of enabled FIFOs in the UART.            }
     NoTimeout          : Boolean;                     {'TRUE' to not enforce "no activity" timeout.    }
     Scrap0             : Byte;                        {n/a                                             }
     PortAddr           : String[3];                   {Hex port address for UART systems.              }
     IRQ                : Byte;                        {IRQ number for the port for UART systems.       }
     Scrap              : Array[1..504] Of Byte;       {n/a                                             }
     END;
{=======================================================================================================}
{ \SHOTGUN\INTRNODE\NODExxxx.$$$                                                                        }
{                                                                                                       }
{  - Online status file for each individual node.                                                       }
{  - xxxx corresponds the individual node number.                                                       }
{  - Single record file.                                                                                }
{                                                                                                       }
{=======================================================================================================}
TYPE _NodeFile_Record        = RECORD
     User                    : String[30];             {The user's real name.                           }
     Alias                   : String[30];             {The user's alias name.                          }
     Baud                    : String[6];              {The user's connect speed with the system.       }
     Location                : String[30];             {The city/province the user is residing.         }
     Activity                : String[40];             {Brief description of the user's activity.       }
     Available               : Boolean;                {'TRUE' if user is available for chat.           }
     PageFlag                : Byte;                   {1-User accepts paging from other nodes.         }
     Node                    : Word;                   {The node the user is logged on to.              }
     END;
{=======================================================================================================}
{ \SHOTGUN\ENGLISH\LANG\OPENING.DAT                                                                     }
{                                                                                                       }
{  - Defines the sequence of events after a user logs on to the system.                                 }
{  - Multiple record file.                                                                              }
{                                                                                                       }
{=======================================================================================================}
TYPE _Opening_Record         = RECORD
     FunctionType            : Byte;                   {Function index for a specific action:           }
                                                       {1-Comment From Last Caller.                     }
                                                       {2-Welcome Screens.                              }
                                                       {3-Once-Only Screens.                            }
                                                       {4-News Screens.                                 }
                                                       {5-Last Callers Screens.                         }
                                                       {6-New Mail Scan.                                }
                                                       {7-New Files Scan.                               }
                                                       {8-Call-Back Verifier.                           }
                                                       {9-User Specific Screen.                         }
                                                       {10-User Access Screen.                          }
                                                       {11-Date Specific Screen.                        }
                                                       {12-User Birthday Screen.                        }
     Forced                  : Byte;                   {1-User is forced to undergo the function.       }
     Sub                     : Word;                   {n/a                                             }
     Area                    : Word;                   {n/a                                             }
     FileName                : String[60];             {Path and file name of display/text files used   }
                                                       {for certain functions.                          }
     END;
{=======================================================================================================}
{ \SHOTGUN\DATA\PROTOCOL.DAT                                                                            }
{                                                                                                       }
{  - Holds toggles for the built in file transfer protocols you use on the system.                      }
{  - Single record file.                                                                                }
{                                                                                                       }
{=======================================================================================================}
TYPE _Protocols_Record       = RECORD
     EnableZ                 : Byte;                   {1-Enable internal ZModem.                       }
     EnableX                 : Byte;                   {1-Enable internal XModem.                       }
     EnableX1k               : Byte;                   {1-Enable internal 1K XModem.                    }
     EnableY                 : Byte;                   {1-Enable internal YModem.                       }
     EnableYG                : Byte;                   {1-Enable internal YModem-G.                     }
     EnableZ8K               : Byte;                   {1-Enable internal ZedZap.                       }
     ZKey                    : Char;                   {Keypress for selecting ZModem.                  }
     XKey                    : Char;                   {Keypress for selecting XModem.                  }
     X1kKey                  : Char;                   {Keypress for selecting 1K XModem.               }
     YKey                    : Char;                   {Keypress for selecting YModem.                  }
     YGKey                   : Char;                   {Keypress for selecting YModem-G.                }
     Z8KKey                  : Char;                   {Keypress for selecting ZedZap.                  }
     END;
{=======================================================================================================}
{ \SHOTGUN\DATA\REPORTS.DAT                                                                             }
{                                                                                                       }
{  - Holds configurations for system generated reports.                                                 }
{  - Multiple record file.                                                                              }
{                                                                                                       }
{=======================================================================================================}
TYPE _Report_Record          = RECORD
     ReportType              : Byte;                   {System report type:                             }
                                                       {1-Tic file received.                            }
                                                       {2-File uploaded by user.                        }
                                                       {3-Private upload received.                      }
                                                       {4-New user on the system.                       }
                                                       {5-File area auto-create.                        }
                                                       {6-Message area auto-create.                     }
     ReportNumber            : Byte;                   {The numerical report number (for reference).    }
     MsgAreaNumber           : Word;                   {AreaID of the msg area the report is to be made.}
     MsgFrom                 : String[30];             {Msg from.                                       }
     MsgTo                   : String[30];             {Msg to.                                         }
     MsgSubject              : String[50];             {Msg subject.                                    }
     Header                  : String[50];             {Path and file name of text file used for the    }
                                                       {report's header.                                }
     Footer                  : String[50];             {Path and file name of text file used for the    }
                                                       {report's footer.                                }
     Scrap                   : Array[1..128] Of Byte;  {n/a                                             }
     END;
{=======================================================================================================}
{ \SHOTGUN\DATA\SCANNERS.DAT                                                                            }
{                                                                                                       }
{  - Holds the configuration for virus scanners used on the system.                                     }
{  - Paths MUST contain a trailing backslash.                                                           }
{  - Single record file.                                                                                }
{                                                                                                       }
{=======================================================================================================}
TYPE _VirusScanners_Record = RECORD
     ScanUploads           : Byte;                     {1-Scan uploaded files.                          }
     Path                  : Array[1..5] Of String[40];{Paths to individual scanners.                   }
     CommandLine           : Array[1..5] Of String[40];{Executable & command line of individual scanners}
     VirusExists           : Array[1..5] Of Byte;      {DOS errorlevel the scanners exit with when a    }
                                                       {virus is found in a file.                       }
     END;
{=======================================================================================================}
{ \SHOTGUN\ENGLISH\SG\xxxxxxxx.SG                                                                       }
{                                                                                                       }
{  - Structure for SG screens used by the SuperVGA interface.                                           }
{  - Due to the fact that a general file structure is used for many very different SuperVGA primitives, }
{    the specifics of this file structure are not given.                                                }
{  - Multiple record file.                                                                              }
{                                                                                                       }
{=======================================================================================================}
TYPE _Raw_File               = RECORD
     Return                  : Char;                   {n/a                                             }
     Number                  : Word;                   {n/a                                             }
     x1                      : Word;                   {n/a                                             }
     y1                      : Word;                   {n/a                                             }
     x2                      : Word;                   {n/a                                             }
     y2                      : Word;                   {n/a                                             }
     Style                   : Word;                   {n/a                                             }
     Colour1                 : Integer;                {n/a                                             }
     Colour2                 : Integer;                {n/a                                             }
     Colour3                 : Integer;                {n/a                                             }
     Colour4                 : Integer;                {n/a                                             }
     Colour5                 : Integer;                {n/a                                             }
     Colour6                 : Integer;                {n/a                                             }
     Colour7                 : Integer;                {n/a                                             }
     Colour8                 : Integer;                {n/a                                             }
     Colour9                 : Integer;                {n/a                                             }
     Colour10                : Integer;                {n/a                                             }
     ObjFile                 : String[12];             {n/a                                             }
     Text                    : String[78];             {n/a                                             }
     Crc32                   : String[8];              {n/a                                             }
     END;
{=======================================================================================================}
{ \SHOTGUN\DATA\SGCONFIG.DAT                                                                            }
{                                                                                                       }
{  - Various settings for SGConfig.                                                                     }
{  - Single record file.                                                                                }
{                                                                                                       }
{=======================================================================================================}
TYPE _SGConfig_Record        = RECORD
     Vertical                : Byte;                   {PixelToMickey for the mouse - y axis.           }
     Horizontal              : Byte;                   {PixelToMickey for the mouse - x axis.           }
     Printer                 : Byte;                   {Printer port of connected printer.              }
     AnsiEditor              : String[40];             {Full path and file name of ANSI screen editor.  }
     RIPEditor               : String[40];             {Full path and file anem of RIP screen editor.   }
     PrinterModel            : Word;                   {Printer make/model:                             }
                                                       {1-Epson.                                        }
                                                       {2-Epson24.                                      }
                                                       {3-Hp150.                                        }
                                                       {4-Hp300.                                        }
                                                       {5-Hp300/Pcl5.                                   }
     Scrap                   : Array[1..940] Of Byte;  {n/a                                             }
     END;
{=======================================================================================================}
{ \SHOTGUN\DATA\SGFPURGE.DAT                                                                            }
{                                                                                                       }
{  - Holds a list of TIC file echos that should be immediately killed on import.                        }
{  - Multiple record file.                                                                              }
{                                                                                                       }
{=======================================================================================================}
TYPE _SGFileKill_Record      = RECORD
     AreaName                : String[12];             {Echo list name of the area to kill.             }
     Scrap                   : Array[1..128] Of Byte;  {n/a                                             }
     END;
{=======================================================================================================}
{ \SHOTGUN\ENGLISH\LANG\SHUTTLE.DAT                                                                     }
{                                                                                                       }
{  - Defines the entries in the shuttle log on menu.                                                    }
{  - Single record file.                                                                                }
{                                                                                                       }
{=======================================================================================================}
TYPE _Shuttle_Record         = RECORD
     KeyPress                : Char;                   {Keypress of the item off of the menu.           }
     Action                  : Byte;                   {Menu function type:                             }
                                                       {1-ANSI interface logon.                         }
                                                       {2-TTY interface logon.                          }
                                                       {3-RIP interface logon.                          }
                                                       {4-SVGA interface logon.                         }
                                                       {5-Download a file.                              }
                                                       {6-Errorlevel exit.                              }
                                                       {7-Hangup.                                       }
                                                       {8-Display Text File.                            }
     ErrorLevel              : Byte;                   {DOS errorlevel to exit with (if applicable).    }
     Text                    : String[40];             {Full path/file name of file to download or      }
                                                       {display (if applicable).                        }
     END;
{=======================================================================================================}
{ \SHOTGUN\DATA\SYSCALLS.DAT                                                                            }
{                                                                                                       }
{  - Information on total calls to the system.                                                          }
{  - Single record file.                                                                                }
{                                                                                                       }
{=======================================================================================================}
TYPE _SystemCalls_Record     = RECORD
     Date_Today              : Longint;                {Today's date - in DOS packed date format.       }
     Total_Calls             : Longint;                {Total calls to the system.                      }
     Today_Calls             : Longint;                {Calls to the system for today.                  }
     END;
{=======================================================================================================}
{ \SHOTGUN\DATA\SYSTEM.DAT                                                                              }
{                                                                                                       }
{  - Holds many global settings that effect the entire system.                                          }
{  - Paths MUST contain a trailing backslash.                                                           }
{  - Single record file.                                                                                }
{                                                                                                       }
{=======================================================================================================}
TYPE _SystemDat_Record       = RECORD
     Sysop                   : String[30];             {The sysop's real name.                          }
     Alias                   : String[30];             {The sysop's alias name (if applicable).         }
     BBS_Name                : String[30];             {The name of the system.                         }
     Location                : String[30];             {The system's location.                          }
     Sys_Idle                : Longint;                {Number of seconds of no activity before the     }
                                                       {system terminates the call.                     }
     Page_Start              : Longint;                {Seconds past midnight that paging the sysop is  }
                                                       {allowed.                                        }
     Page_End                : Longint;                {Seconds past midnight that paging the sysop is  }
                                                       {no longer allowed.                              }
     Page_Max                : Byte;                   {Maximum number of sysop pages a user can do in  }
                                                       {one online session.                             }
     DateFormat              : Byte;                   {Date format to use:                             }
                                                       {1-MM/DD/YY                                      }
                                                       {2-DD/MM/YY                                      }
     Show_Empty              : Byte;                   {n/a                                             }
     CD_Roms                 : Byte;                   {1-Enable special handling of CD rom areas.      }
     Use_CD_Temp             : Byte;                   {1-Use disk paging for CD rom areas.             }
     Access_New_User         : Word;                   {Access level to give new callers to the system. }
     TBank_Max               : Longint;                {Maximum minutes allowed in the time bank.       }
     BBank_Max               : Longint;                {Maximum bytes allowed in the byte bank.         }
     TBank_Trans             : Longint;                {Maximum number of minutes allowed to be taken   }
                                                       {from the time bank in one calendar day.         }
     BBank_Trans             : Longint;                {Maximum number of bytes allowed to be traken    }
                                                       {from the byte bank in one calendar day.         }
     Alias_System            : Boolean;                {'TRUE' to allow the use of alias names.         }
     PrivateRead             : Word;                   {Access level required to read other people's    }
                                                       {private messages.                               }
     EchoExit                : Byte;                   {DOS errorlevel that SG will exit with when echo }
                                                       {mail is created on the system.                  }
     LocalExit               : Byte;                   {DOS errorlevel that SG will exit with when local}
                                                       {mail is created on the system.                  }
     FileListName            : String[8];              {Filename (no extension) of file lists that are  }
                                                       {generated online by the users.                  }
     MessageDebug            : Byte;                   {1-Display message kludge information locally.   }
     Suspend_FList           : Byte;                   {1-Disable user's time when transferring QWK etc.}
     MessageCarbonCopy       : Word;                   {Access level required to create carbon copy msgs}
     MessageCopy             : Word;                   {Access level required to copy messages in bases.}
     MessageMove             : Word;                   {Access level required to move messages in bases.}
     MessageForward          : Word;                   {Access level required to forward messages in    }
                                                       {bases.                                          }
     MessageKill             : Word;                   {Access level required to kill messages in bases.}
     FileMove                : Word;                   {Access level required to move files in areas.   }
     FileCopy                : Word;                   {Access level required to copy files in areas.   }
     FileKill                : Word;                   {Access level required to kill files in areas.   }
     ANSIExit                : Byte;                   {DOS errorlevel to exit with when a user selects }
                                                       {a non-SuperVGA interface (for those that use    }
                                                       {multiple BBS packages.                          }
     BadQWK                  : String[40];             {Path to place astray/bad QWK packets.           }
     FileAttach              : String[40];             {Path to place outbound netmail attached files.  }
     MessageInfo             : Word;                   {Access level required to see msg kludge lines.  }
     Scrap                   : Array[1..39] Of Byte;   {n/a                                             }
     CheckDupes              : Byte;                   {1-Check for dupes during upload.                }
     CheckDupesCD            : Byte;                   {1-Check for dupes on CD Roms during upload.     }
     QWKPacketName           : String[8];              {Filename (no extension) of QWK packets that are }
                                                       {generated online by the users.                  }
     QWKBBSId                : String[60];             {Your BBS ID (origin line) for return QWK packets}
     QWKBBSPhone             : String[20];             {The system's main phone number.                 }
     BadFiles                : String[40];             {Path to place bad/infected uploaded files.      }
     NodeDrive               : String[1];              {Hard drive partition the user's node directories}
                                                       {should be created.                              }
     SwapMethod              : Byte;                   {Swapping method used by Shotgun:                }
                                                       {1-EMS/XMS.                                      }
                                                       {2-EMS/XMS/DISK.                                 }
                                                       {3-DISK.                                         }
     END;
{=======================================================================================================}
{ \SHOTGUN\SGMAIL\NODELIST\USERINDX.SG                                                                  }
{                                                                                                       }
{  - Sysop index for an entry in the NODELIST.SG file.                                                  }
{  - Multiple record file.                                                                              }
{                                                                                                       }
{=======================================================================================================}
TYPE _USERINDX_SG            = RECORD
     Index                   : Word;                   {Pointer to position (zero based) in NODELIST.SG.}
     Sysop                   : String[25];             {The sysop's name.                               }
     END;
{=======================================================================================================}
{ \SHOTGUN\DATA\USERS.BBS                                                                               }
{                                                                                                       }
{  - The user base file.                                                                                }
{  - The first record is not a user. It is only referenced for the User_Number variable which tells     }
{    the system what user number the next new user gets when they call the system.                      }
{  - Multiple record file.                                                                              }
{                                                                                                       }
{=======================================================================================================}
TYPE _UsersBBS_Record        = RECORD
     Name                    : String[30];             {User's real name.                               }
     User_Number             : Longint;                {User's user number.                             }
     Alias                   : String[30];             {User's alias name.                              }
     Password                : String[20];             {User's password.                                }
     Security                : Word;                   {User's access level.                            }
     Voice                   : String[20];             {User's voice phone number.                      }
     Data                    : String[20];             {User's data phone number.                       }
     Birth                   : String[6];              {User's birthdate (MM/DD/YY).                    }
     Street                  : String[30];             {User's street address.                          }
     City                    : String[30];             {User's city.                                    }
     Prov                    : String[30];             {User's province/state.                          }
     Country                 : String[30];             {User's country.                                 }
     Postal                  : String[20];             {User's postal code.                             }
     Sex                     : Byte;                   {0-Female, 1-Male.                               }
     BBS                     : String[30];             {Name of the user's BBS.                         }
     BBS_Phone               : String[20];             {Phone number of the user's BBS.                 }
     BBS_Net                 : String[20];             {Network address of the user's BBS.              }
     Page                    : Byte;                   {1-User can page sysop.                          }
     Tags                    : Array[1..10] Of Byte;   {User's access tags.                             }
     Note                    : String[60];             {Description of the user.                        }
     Seperator               : Byte;                   {1-Use seperator lines in file areas.            }
     File_Lines              : Byte;                   {Number of file description lines to display.    }
     Uploads                 : Longint;                {Number of bytes uploaded in total.              }
     DLBytesToday            : Longint;                {Number of bytes downloaded today.               }
     DL_Total                : Longint;                {Number of bytes downloaded in total.            }
     Files_Uploaded          : Longint;                {Number of files uploaded in total.              }
     Files_Downloaded        : Longint;                {Number of files downloaded in total.            }
     File_Ratio              : Longint;                {Current UL:DL ratio.                            }
     Byte_Ratio              : Longint;                {Current byte ratio.                             }
     Daily_Files             : Longint;                {n/a                                             }
     DLBytesRemaining        : Longint;                {Number of bytes the user can still DL today.    }
     First_Call              : Longint;                {User's first call - in DOS packed date format.  }
     Last_Call               : Longint;                {User's last call - in DOS packed date format.   }
     Total_Calls             : Longint;                {Total calls the user has made to the system.    }
     SecondsRemaining        : Longint;                {Seconds remaining for today's sessions.         }
     Sub_Expiry              : Longint;                {Expiry date of subscription.                    }
     Expiry_Adjust           : Word;                   {Access level to be demoted to after expiry.     }
     Punt                    : Byte;                   {1-User is locked out from the system.           }
     DefaultArchiver         : Word;                   {Default Archiver - 1 based to ARCUTILS.DAT.     }
     DefaultProtocol         : Word;                   {Default Protocol:                               }
                                                       {1-ZModem                                        }
                                                       {2-XModem                                        }
                                                       {3-XModem1K                                      }
                                                       {4-YModem                                        }
                                                       {5-YModemG                                       }
                                                       {Other-No default selected                       }
     SubMinutesUsed          : Longint;                {Minutes used of the user's subscription.        }
     Scrap1                  : Array[1..2] Of Longint; {n/a                                             }
     UserList                : Byte;                   {1-User is shown in online userlist.             }
     PageFlag                : Byte;                   {1-User is available for page.                   }
     IBM                     : Byte;                   {1-User has IBM character capability.            }
     ANSI                    : Byte;                   {1-User has ANSI graphics capability.            }
     Msg_Editor              : Byte;                   {0-Line editor, 1-Full screen editor.            }
     Scrap2                  : Word;                   {n/a                                             }
     Clear_Scrn              : Byte;                   {1-Screen clear codes are desired.               }
     Bad_Logon               : Byte;                   {1-Last logon to the system failed.              }
     Ver_User                : Byte;                   {1-User is verfied through the CBV.              }
     SecondsInBank           : Longint;                {Seconds stored in the time bank.                }
     ByteBankBalance         : Longint;                {Bytes stored in the byte bank.                  }
     TimeBankWithdrawls      : Longint;                {Seconds taken from the time bank today.         }
     ByteBankWithdrawls      : Longint;                {Bytes taken from the byte bank today.           }
     SubModel                : Byte;                   {Subscription model:                             }
                                                       {0-None                                          }
                                                       {1-By calendar days                              }
                                                       {2-By call days                                  }
                                                       {3-By minutes used                               }
                                                       {4-Specific date                                 }
     SubCalendarDaysUsed     : Longint;                {Calendar days used for subscription.            }
     SubCallDaysUsed         : Longint;                {Call days used for subscription.                }
     SubLastChange           : Longint;                {Last date subscription changed - in DOS packed  }
                                                       {date format.                                    }
     END;
{=======================================================================================================}
{ \SHOTGUN\DATA\VERIFY.DAT                                                                              }
{                                                                                                       }
{  - Holds all the user's names & phone numbers that have been through the CBV - for dupe checking.     }
{  - Multiple record file.                                                                              }
{                                                                                                       }
{=======================================================================================================}
TYPE _VerifyLog_Record       = RECORD
     Name                    : String[30];             {The user's real name.                           }
     Number                  : String[20];             {The phone number that was dialed.               }
     END;
{=======================================================================================================}
{ xxxxxxxx.GIP (1)                                                                                      }
{                                                                                                       }
{  - This is the structure of the GIP file's main information header.                                   }
{                                                                                                       }
{=======================================================================================================}
TYPE _GIPHeader              = RECORD
     Title                   : String[30];             {Title of the GIP file.                          }
     Author                  : String[30];             {The name of the author of the GIP.              }
     Area                    : String[30];             {The area the GIP is to be stored.               }
     Date                    : Longint;                {Date of GIP creation - in DOS packed date format}
     END;
{=======================================================================================================}
{ xxxxxxxx.GIP (2)                                                                                      }
{                                                                                                       }
{  - This is the structure of the header for individually packed files in the GIP file.                 }
{                                                                                                       }
{=======================================================================================================}
TYPE _GIPFileHeader          = RECORD
     FName                   : String[12];             {The file name of the packed file.               }
     FileLength              : Longint;                {The file length of the packed file.             }
     END;
{=======================================================================================================}
{ \SHOTGUN\DATA\FREQ.DAT                                                                                }
{                                                                                                       }
{  - Configuration for offline file requesting.                                                         }
{  - Single record file.                                                                                }
{                                                                                                       }
{=======================================================================================================}
TYPE _FreqRecord             = RECORD
     AllowFreqs              : Boolean;                {'TRUE' to allow users to request offline files. }
     DaysToKeepRequests      : Byte;                   {Number of days the system will try to fill the  }
                                                       {request before it is erased.                    }
     Access                  : Word;                   {Minimum access level required to freq files.    }
     Tags                    : Array[1..10] Of Byte;   {User tags required to freq files.               }
     SysopMessageAreaID      : Word;                   {AreaID of msg area used for note to the sysop.  }
     SysopMessageTo          : String[30];             {'Msg To' field for the node sent to the sysop.  }
     END;
{=======================================================================================================}
{ \SHOTGUN\ENGLISH\LANG\xxxxxxxx.CMD                                                                    }
{                                                                                                       }
{  - Commands used for key presses and strings for various system screens.                              }
{  - File name corresponds with the associated system screen.                                           }
{  - Multiple record file.                                                                              }
{                                                                                                       }
{=======================================================================================================}
TYPE _Command                = RECORD
     KeyPress                : Char;                   {Key press for the command.                      }
     ANSIString              : String[40];             {String displayed to the user in text mode.      }
     END;


IMPLEMENTATION


END.
