<form id="add-group" method="post" action="<?php echo $_SERVER['PHP_SELF']; ?>">
	<h3>LDAP Group Creation Utility</h3>
	Group Name: <input name="cn" type="text" maxlength="255" value=""/> 
	<br />

	<input id="submit" type="submit" name="submit" value="Create LDAP Group" />
</form>	
