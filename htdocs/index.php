<!DOCTYPE HTML PUBLIC "-//IETF//DTD HTML 3.2//EN">
<html>
<head>
  <title>LATS - Look at the stars - an image-viewer for linux</title>
  <link rel="stylesheet" href="lats.css" type="text/css">
</head>

<body 
  bgcolor="#FFFFFF"
  text="#000000"
  link="#0000FF"
  vlink="#FFFFFF"
  alink="#FFFF00"
>

<table width=100% border=0 xxxcellpadding=0 xxxcellspacing=0 style="position:absolute ; left:0 ; top:0 ;">
  <tr>
    <td colspan=3>
      <?php include ("titelbar.html") ; ?>
    </td>
  </tr>
  <tr>
    <?php
      if ( isset($extern) )
      {
        echo "    <td valign=top colspan=3>" ;
        include ($extern) ;
        echo "    </td>" ;
      } else if ( isset($pic) )
      {
        echo "    <td valign=middle colspan=3>" ;
	echo "      <center><font color=black>";
	echo "      <a href=javascript:history.back() >go back</a><br>";
	echo "      <br>";
	echo "      <img src=screenshots/$pic ><br>";
	echo "      <br>";
	echo "      <a href=javascript:history.back() >go back</a><br><br>";
	echo "      </font></center>";
        echo "    </td>" ;
      } else {
        echo "    <td valign=top>";
        include ("left_pane.html") ;
        echo "    </td>";
        echo "    <td valign=top width=60%>";
        include ("body.html") ;
        echo "    </td>";
        echo "    <td valign=top>";
        include ("right_pane.html") ;
        echo "    </td>";
      }
    ?>
  </tr>
</table>

</body>
</html>
