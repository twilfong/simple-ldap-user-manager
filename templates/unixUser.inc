<?php


// Available variables are:
// $uid 

$attributes['gidNumber'] = 2017;
$attributes['loginShell'] = "/bin/bash";
$attributes['objectClass'][0] = "top";
$attributes['objectClass'][1] = "inetOrgPerson";
$attributes['objectClass'][2] = "posixAccount";


$attributes['cn'] = $uid;
$attributes['sn'] = $uid;
$attributes['homeDirectory'] = '/home/' . $uid;



?>
