<?php

$LDAPDN = "dc=example,dc=com";
$LDAPURL = "ldapi:///";
// $LDAPURL = "ldaps://some.remote.server/";
$UIDBASE = "ou=People,$LDAPDN";

$FULL_NAME_ATTR = 'cn';

// array of LDAP attributes to allow a non-admin user to modify
// elements should be 'attribute' => 'description'
$LDAP_ATTRS = array(
    'mobile' => 'Mobile Numer',
    'homephone' => 'Home or Alternate Phone'
);

$LDAPPASSWD_CMD = "/usr/local/bin/ldappasswd-wrapper $LDAPURL";

$MSG_BG_COLORS = array(
    '#99d0ff',	# Bg color for information/success
    '#ffd0d0'	# Bg color for error/warning
);
