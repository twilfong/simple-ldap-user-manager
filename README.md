![Lightweight LDAP Account Management API](https://raw.githubusercontent.com/integrii/LLAMA/master/img/llamas.gif)


## DESCRIPTION
This application provides a generic but flexible LDAP API that can be used
easily with anything that can make a GET or POST request. This application
makes it trivial to write automation against LDAP.


## SETUP
Copy config.inc.example to config.inc and configure for your environment.


## Requirements

You need the php-ldap package. `yum install php-ldap -y`

## USAGE

There are different endpoints for adding users, groups, removing users, modifying users, viewing users and adding users to groups...  Just form a URL and call it with wget or curl.

Example: `wget -O - -q "http://localhost/add-to-group.php/?uid=testuser&cn=testgroup"`


### Creating Users
* **URL:** http://localhost/add-user.php
* **Returns:** uidNumber of created account (plaintext)
* **GET/POST Parameters:** uid,password,template,attributes

    * **uid:** Username (string plaintext) (Required)
    * **password:** Password (string plaintext) (Optional)
    * **template:** unixUser or ftpAccount (Required)
        * unixUser
            * required attributes: gidNumber (defaults to 1000)
            * optional attributes: uidNumber,homeDirectory
        * ftpAccount
            * required attributes: gidNumber (defaults to 1000)
            * optional attributes: uidNumber,,homeDirectory
            * attributes: LDAPAttribute=Value,LDAPAttribute=Value... (string plaintext) (Optional)
    * gidNumber (optional - defaults to 1000)
    * uidNumber (optional - determines next automatically if not set)

**EXAMPLE:** `wget "http://localhost/add-user.php?uid=test&password=1234&template=unixUser&attributes=homeDirectory=/home/testuser,gidNumber=7000" -q -O -`

### Creating Groups
* **URL:** http://localhost/add-group.php
* **Returns:** gidNumber of created account (plaintext)
* **GET/POST Parameters:** cn,attributes

    * **cn:** Group Name (string plaintext) (Required)
    * **attributes:** LDAPAttribute=Value,LDAPAttribute=Value... (string plaintext) (Optional)
        * gidNumber (defaults to 1000)

**EXAMPLE:** `wget "http://localhost/add-group.php?cn=test&attributes=gidNumber=5000" -q -O -`


### Adding User to Group #
* **URL:** http://localhost/add-to-group.php
* **Returns:** nothing
* **GET/POST Parameters:** cn,uid

    * **cn:** Group Name (string plaintext) (Required)
    * **uid:** User Name (string plaintext) (Required)


### Removing User from Group
* **URL:** http://localhost/remove-from-group.php
* **Returns:** nothing
* **GET/POST Parameters:**  cn,uid

    * **cn:** Group Name (string plaintext) (Required)
    * **uid:** User Name (string plaintext) (Required)


### View user attributes
* **URL:** http://localhost/view-user.php
* **Returns:** php print_r of user's attributes
* **GET/POST Parameters:**  uid

    * **uid:** User Name (string plaintext) (Required)

### Edit user attributes
* **URL:** http://localhost/modify-user.php
* **Returns:** nothing
* **GET/POST Parameters:**  uid, attributes

    * **uid:** User Name (string plaintext) (Required)
    * **attributes:** LDAPAttribute=Value,LDAPAttribute=Value... (string plaintext) (Optional)

**EXAMPLE:** `wget "http://localhost/modify-user.php?uid=test&attributes=homeDirectory=/home/testuser,gidNumber=7000" -q -O -`
