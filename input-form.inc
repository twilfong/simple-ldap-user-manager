<form id="add-user" method="post" action="<?php echo $_SERVER['PHP_SELF']; ?>">
	<h3>LDAP Account Creation Utility</h3>
	Username: <input name="uid" type="text" maxlength="255" value=""/> 
	<br />

	Template: 
	<select name="template">
		<option value="unixUser">Unix User</option>
		<option value="asperaUser">Aspera User</option>
		<option value="ftpAccount">FTP Account</option>
	</select>
	<br />

	Password:
	<input type="password" name="password">
	<br />


	<input id="submit" type="submit" name="submit" value="Create LDAP User" />
</form>	
