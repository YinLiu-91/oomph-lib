#!MC 1120

$!VARSET |PNG|=0

$!VarSet |dir| = 'RESLT_LOAD_BALANCE'
$!VarSet |postfix| = '.plt'
$!VarSet |postfix| = '.dat'


$!VARSET |nproc|=2

$!VARSET |lostep|=0
$!VARSET |nstep|=12
$!VARSET |dstep|=1

$!NEWLAYOUT 
$!IF |PNG|==0
     $!EXPORTSETUP EXPORTFORMAT = AVI
     $!EXPORTSETUP IMAGEWIDTH = 806
     $!EXPORTSETUP EXPORTFNAME = 'prune_and_balance.avi'
     $!EXPORTSTART             
       EXPORTREGION = ALLFRAMES 
$!ENDIF             

$!LOOP |nstep|
$!VARSET |step|=(|lostep|+(|loop|-1)*|dstep|)
$!NEWLAYOUT 
$!DRAWGRAPHICS FALSE

$!FIELDLAYERS SHOWSHADE = YES

$!LOOP |nproc|

$!IF |LOOP|==1
   $!VARSET |n2_0|=1
$!ELSE
   $!VARSET |n2_0|=(|NUMZONES|+1)
$!ENDIF

$!VARSET |proc|=(|LOOP|-1)

$!READDATASET  '"|dir|/soln|step|_on_proc|proc||postfix|" '
  READDATAOPTION = APPEND
  RESETSTYLE = NO
  INCLUDETEXT = YES
  INCLUDEGEOM = NO
  INCLUDECUSTOMLABELS = NO
  VARLOADMODE = BYNAME
  ASSIGNSTRANDIDS = YES
  INITIALPLOTTYPE = CARTESIAN2D
  VARNAMELIST = '"V1" "V2" "V3" "V4" "V5"'
$!VARSET |n2|=|NUMZONES|
$!FIELDMAP [|n2_0|-|n2|]  GROUP = 12
$!IF |proc|==0
   $!FIELDMAP [|n2_0|-|n2|]  EDGELAYER{COLOR = BLUE}
$!ELSEIF |proc|==1
   $!FIELDMAP [|n2_0|-|n2|]  EDGELAYER{COLOR = RED}
$!ELSEIF |proc|==2
   $!FIELDMAP [|n2_0|-|n2|]  EDGELAYER{COLOR = GREEN}
$!ELSEIF |proc|==3
   $!FIELDMAP [|n2_0|-|n2|]  EDGELAYER{COLOR = PURPLE}
$!ENDIF
$!FIELDMAP [|n2_0|-|n2|]  MESH{SHOW = NO}
$!ACTIVEFIELDMAPS += [|n2_0|-|n2|]
$!REDRAWALL 
$!FIELDMAP [|n2_0|-|n2|]  EDGELAYER{LINETHICKNESS = 0.100000000000000006}


$!ENDLOOP


$!LOOP |nproc|

$!VARSET |proc|=(|LOOP|-1)

$!VARSET |n4_0|=(|NUMZONES|+1)
$!READDATASET  '"|dir|/halo_elements_on_proc|proc|_|step||postfix|" '
  READDATAOPTION = APPEND
  RESETSTYLE = NO
  INCLUDETEXT = NO
  INCLUDEGEOM = NO
  INCLUDECUSTOMLABELS = NO
  VARLOADMODE = BYNAME
  ASSIGNSTRANDIDS = YES
  INITIALPLOTTYPE = CARTESIAN2D
  VARNAMELIST = '"V1" "V2" "V3" "V4" "V5"'
$!VARSET |n4|=|NUMZONES|
$!FIELDMAP [|n4_0|-|n4|]  GROUP = 14
$!ACTIVEFIELDMAPS += [|n4_0|-|n4|]
$!FIELDMAP [|n4_0|-|n4|]  MESH{SHOW = NO}
$!FIELDMAP [|n4_0|-|n4|]  EDGELAYER{SHOW = NO}
$!FIELDMAP [|n4_0|-|n4|]  SHADE{COLOR = CYAN}

$!ENDLOOP


$!LOOP |nproc|

$!VARSET |proc|=(|LOOP|-1)

$!FIELDLAYERS SHOWSCATTER = YES
$!FIELDMAP [1-|n4|]  SCATTER{SHOW = NO}

$!VARSET |n5_0|=(|NUMZONES|+1)
$!READDATASET  '"|dir|/halo_nodes_on_proc|proc|_|step||postfix|" '
  READDATAOPTION = APPEND
  RESETSTYLE = NO
  INCLUDETEXT = NO
  INCLUDEGEOM = NO
  INCLUDECUSTOMLABELS = NO
  VARLOADMODE = BYPOSITION
  ASSIGNSTRANDIDS = YES
  INITIALPLOTTYPE = CARTESIAN2D
$!VARSET |n5|=|NUMZONES|
$!ACTIVEFIELDMAPS += [|NUMZONES|]
$!FIELDMAP [|NUMZONES|]  SCATTER{SHOW = YES}
$!IF |proc|==0
   $!FIELDMAP [|NUMZONES|]  SCATTER{COLOR =BLUE}
$!ELSEIF |proc|==1
   $!FIELDMAP [|NUMZONES|]  SCATTER{COLOR =RED}
$!ELSEIF |proc|==2
   $!FIELDMAP [|NUMZONES|]  SCATTER{COLOR =GREEN}
$!ELSEIF |proc|==3
   $!FIELDMAP [|NUMZONES|]  SCATTER{COLOR =PURPLE}
$!ENDIF
$!FIELDMAP [|NUMZONES|]  SCATTER{FRAMESIZE = 0.5}
$!FIELDMAP [|NUMZONES|]  MESH{SHOW = NO}

$!ENDLOOP


###################################################
$!VIEW FIT

$!DRAWGRAPHICS TRUE
$!REDRAWALL


$!IF |PNG|==1
     $!EXPORTSETUP EXPORTFORMAT = PNG
     $!EXPORTSETUP IMAGEWIDTH = 600
     $!EXPORTSETUP EXPORTFNAME = 'prune_and_balance|loop|.png'
     $!EXPORT
       EXPORTREGION = ALLFRAMES
$!ELSE
     $!EXPORTNEXTFRAME
$!ENDIF

$!ENDLOOP

$!IF |PNG|==0
$!EXPORTFINISH
$!ENDIF
