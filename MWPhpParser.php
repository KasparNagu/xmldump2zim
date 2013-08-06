<?php
//some infrastuctore for the parse to run standalone
define( 'MEDIAWIKI', TRUE);
$wgCommandLineMode = true;

$IP="../mediawiki-1.21.1";

require_once("$IP/includes/Defines.php");
require_once("$IP/includes/profiler/Profiler.php");
require_once("$IP/includes/GlobalFunctions.php");
require_once("$IP/includes/Init.php");
require_once("$IP/includes/AutoLoader.php");
require_once("$IP/includes/DefaultSettings.php");
require_once( "$IP/extensions/ParserFunctions/ParserFunctions.php");
require_once( "$IP/extensions/Cite/Cite.php");
require_once( "$IP/extensions/Math/Math.php");

require_once( "$IP/includes/normal/UtfNormalUtil.php" );

$self="MWPhpParser";
$wgLocaltimezone="UTC";
$wgInvalidateCacheOnLocalSettingsChange=false;
$wgScriptPath="$IP";
$wgPreprocessorCacheThreshold=False;
$wgUploadPath = $wgUploadDirectory = "images";
$wgArticlePath="/A/$1"; #/A/ will be the namespaced used in the zim file
$wgDBtype = 'fake';

require_once( "$IP/includes/Setup.php");

//$wgLang = $wgContLang = Language::factory( "en" );


$wgLocalFileRepo = array(
        'class' => 'LocalRepo',
        'name' => 'local',
        'directory' => $wgUploadDirectory,
        'scriptDirUrl' => $wgScriptPath,
        'scriptExtension' => $wgScriptExtension,
        'url' => $wgUploadBaseUrl ? $wgUploadBaseUrl . $wgUploadPath : $wgUploadPath,
        'hashLevels' => $wgHashedUploadDirectory ? 2 : 0,
        'thumbScriptUrl' => $wgThumbnailScriptPath,
        'transformVia404' => !$wgGenerateThumbnailOnParse,
        'deletedDir' => $wgDeletedDirectory,
        'deletedHashLevels' => 3,
	'backend' => 'local-backend'
);

$wgLockManagers[] = array(
        'name'          => 'fsLockManager',
        'class'         => 'FSLockManager',
        'lockDirectory' => "{$wgUploadDirectory}/lockdir",
);
$wgLockManagers[] = array(
        'name'          => 'nullLockManager',
        'class'         => 'NullLockManager',
);


class DatabaseFake extends DatabaseBase {
	function debugOut($str){
#		echo($str);echo("\n");
#		print_stack_trace();

	}
	function getType() {return "fake";}

	function open( $server, $user, $password, $dbName ){
		$this->debugOut("FakeDatabase::open($server,$user,$password,$dbName)");
		$this->mOpened = True;
		return True;
	}

	/**
	 * Fetch the next row from the given result object, in object form.
	 * Fields can be retrieved with $row->fieldname, with fields acting like
	 * member variables.
	 * If no more rows are available, false is returned.
	 *
	 * @param $res ResultWrapper|object as returned from DatabaseBase::query(), etc.
	 * @return object|bool
	 * @throws DBUnexpectedError Thrown if the database returns an error
	 */
	function fetchObject( $res ) {
		if($res instanceof  FakeResultWrapper){
			$this->debugOut("FakeDatabase::fetchObject(FakeResultWrapper)");
			return $res->fetchObject();
		}
		$this->debugOut("FakeDatabase::fetchObject(res)");
		return False;
	}

	/**
	 * Fetch the next row from the given result object, in associative array
	 * form.  Fields are retrieved with $row['fieldname'].
	 * If no more rows are available, false is returned.
	 *
	 * @param $res ResultWrapper result object as returned from DatabaseBase::query(), etc.
	 * @return array|bool
	 * @throws DBUnexpectedError Thrown if the database returns an error
	 */
	function fetchRow( $res ){
		if($res instanceof  FakeResultWrapper){
			$this->debugOut("FakeDatabase::fetchRow(FakeResultWrapper)");
			return $res->fetchRow();
		}
		$this->debugOut("FakeDatabase::fetchRow(res)");
		return False;
	}

	/**
	 * Get the number of rows in a result object
	 *
	 * @param $res Mixed: A SQL result
	 * @return int
	 */
	function numRows( $res ){
		if($res instanceof FakeResultWrapper){
			$this->debugOut("FakeDatabase::numRows(FakeResultWrapper)");
			return $res->numRows();
		}
		$this->debugOut("FakeDatabase::numRows(res)");
		return 0;
	}

	/**
	 * Get the number of fields in a result object
	 * @see http://www.php.net/mysql_num_fields
	 *
	 * @param $res Mixed: A SQL result
	 * @return int
	 */
	function numFields( $res ){
		$this->debugOut("FakeDatabase::numFields(res)");
		return 0;
	}

	/**
	 * Get a field name in a result object
	 * @see http://www.php.net/mysql_field_name
	 *
	 * @param $res Mixed: A SQL result
	 * @param $n Integer
	 * @return string
	 */
	function fieldName( $res, $n ){
		$this->debugOut("FakeDatabase::fieldName(res,$n)");
		return "noname";
	}

	/**
	 * Get the inserted value of an auto-increment row
	 *
	 * The value inserted should be fetched from nextSequenceValue()
	 *
	 * Example:
	 * $id = $dbw->nextSequenceValue( 'page_page_id_seq' );
	 * $dbw->insert( 'page', array( 'page_id' => $id ) );
	 * $id = $dbw->insertId();
	 *
	 * @return int
	 */
	function insertId(){
		$this->debugOut("FakeDatabase::insertedId(res)");
		return 0;
	}

	/**
	 * Change the position of the cursor in a result object
	 * @see http://www.php.net/mysql_data_seek
	 *
	 * @param $res Mixed: A SQL result
	 * @param $row Mixed: Either MySQL row or ResultWrapper
	 */
	function dataSeek( $res, $row ){
		$this->debugOut("FakeDatabase::dataSeek(res,$row)");
		return null;
	}

	/**
	 * Get the last error number
	 * @see http://www.php.net/mysql_errno
	 *
	 * @return int
	 */
	function lastErrno(){
		$this->debugOut("FakeDatabase::lastErrno()");
		return 0;
	}

	/**
	 * Get a description of the last error
	 * @see http://www.php.net/mysql_error
	 *
	 * @return string
	 */
	function lastError(){
		$this->debugOut("FakeDatabase::lastError()");
		return "none";
	}

	/**
	 * mysql_fetch_field() wrapper
	 * Returns false if the field doesn't exist
	 *
	 * @param string $table table name
	 * @param string $field field name
	 *
	 * @return Field
	 */
	function fieldInfo( $table, $field ){
		$this->debugOut("FakeDatabase::fieldInfo($table,$field)");
		return false;
	}

	/**
	 * Get information about an index into an object
	 * @param string $table Table name
	 * @param string $index Index name
	 * @param string $fname Calling function name
	 * @return Mixed: Database-specific index description class or false if the index does not exist
	 */
	function indexInfo( $table, $index, $fname = 'Database::indexInfo' ){
		$this->debugOut("FakeDatabase::indexInfo($table,$index,$fname)");
		return false;
	}

	/**
	 * Get the number of rows affected by the last write query
	 * @see http://www.php.net/mysql_affected_rows
	 *
	 * @return int
	 */
	function affectedRows(){
		$this->debugOut("FakeDatabase::affectedRows()");
		return 0;
	}

	/**
	 * Wrapper for addslashes()
	 *
	 * @param string $s to be slashed.
	 * @return string: slashed string.
	 */
	function strencode( $s ){
		$this->debugOut("FakeDatabase::strencode($s)");
		return str_replace('\'','\\\'',$s);
	}

	/**
	 * Returns a wikitext link to the DB's website, e.g.,
	 *     return "[http://www.mysql.com/ MySQL]";
	 * Should at least contain plain text, if for some reason
	 * your database has no website.
	 *
	 * @return string: wikitext of a link to the server software's web site
	 */
	static function getSoftwareLink(){
		$this->debugOut("FakeDatabase::getSoftwareLink()");
		return "none";
	}

	/**
	 * A string describing the current software version, like from
	 * mysql_get_server_info().
	 *
	 * @return string: Version information from the database server.
	 */
	function getServerVersion(){
		$this->debugOut("FakeDatabase::getServerVersion()");
		return "none";
	}

	/**
	 * A string describing the current software version, and possibly
	 * other details in a user-friendly way.  Will be listed on Special:Version, etc.
	 * Use getServerVersion() to get machine-friendly information.
	 *
	 * @return string: Version information from the database server
	 */
	function getServerInfo(){
		$this->debugOut("FakeDatabase::getServerInfo()");
		return "none";
	}        

	protected function closeConnection(){
		$this->debugOut("FakeDatabase::closeConnection()");
		return true;
	}

	protected function doQuery( $sql ){		
		$this->debugOut("FakeDatabase::doQuery($sql)");
		return new ResultWrapper($this,$this);

	}	
	protected $i=1;
	protected $templateIdCounter=1;
	protected $templateIdToName = array();
	public function query( $sql, $fname = '', $tempIgnore = false ){
		$this->debugOut("FakeDatabase::query($sql, $fname,$tempIgnore)");
		$result = False;
		if(preg_match('/SELECT\s*rev_id,rev_page,rev_text_id,rev_timestamp,rev_comment,rev_user_text,rev_user,rev_minor_edit,rev_deleted,rev_len,rev_parent_id,rev_sha1,rev_content_format,rev_content_model,page_namespace,page_title,page_id,page_latest,page_is_redirect,page_len,user_name  FROM "revision" INNER JOIN "page" ON \(\(page_id = rev_page\)\) LEFT JOIN "user" ON \(\(rev_user != 0\) AND \(user_id = rev_user\)\)\s*WHERE page_namespace = \'10\' AND page_title = \'((?:[^\']|\\\')*)\' AND \(rev_id=page_latest\)\s+LIMIT 1/',$sql,$matches)){
#			echo "get template $matches[1]\n";
			$this->templateIdToName[$this->templateIdCounter] = str_replace("_"," ",$matches[1]);
			$result=array(array(
				"rev_id"=>$this->templateIdCounter,
				"rev_page"=>$this->templateIdCounter,
				"rev_text_id"=>$this->templateIdCounter++,
				"rev_timestamp"=>False,
				"rev_comment"=>"",
				"rev_user_text"=>"",
				"rev_user"=>"",
				"rev_minor_edit"=>False,
				"rev_deleted"=>False,
				"rev_len"=>False,
				"rev_parent_id"=>False,
				"rev_sha1"=>False,
				"rev_content_format"=>False,
				"rev_content_model"=>False,
				"page_namespace"=>False,
				"page_title"=>False,
				"page_id"=>False,
				"page_latest"=>False,
				"page_is_redirect"=>False,
				"page_len"=>False,
				"user_name"=>""));
		}else if(preg_match('/SELECT\s*old_text,old_flags\s*FROM "text"\s*WHERE old_id = \'([^\']*)\'\s*LIMIT 1/',$sql,$matches)){
			$title = $this->templateIdToName[$matches[1]];
#			echo "get text for $title ($matches[1])\n";
			$result = array(array("old_text"=>getArticleText("Vorlage:".$title),"old_flags"=>"utf-8"));
		}else if(preg_match('/SELECT\s*page_id\s*FROM "page"\s*WHERE page_namespace = \'6\' AND page_title =\'((?:[^\']|\\\')*)\'\s+LIMIT 1/',$sql,$matches)){
			echo "get image $matches[1]\n";
		}else if(preg_match('/SELECT\s*page_id,page_len,page_is_redirect,page_latest,page_content_model\s*FROM "page"\s*WHERE\s*\(?page_namespace = \'([^\']*)\' AND page_title = \'((?:[^\']|\\\')*)\'\s*\)?(?:\s*LIMIT 1)?/',$sql,$matches)){
			#we serve a fake id, so every page is defined
			$result = array(array('page_id'=>$this->i++, 'page_len'=>10, 'page_is_redirect'=>False, 'page_latest'=>True, 'page_content_model'=>False));
		}else if(strpos($sql,"math")!==False){
			//todo
		}else if(strpos($sql,"l10n_cache")===False){
			echo "unhandled: $sql\n";
		}
		if($result !== False){
			return new FakeResultWrapper2($result);
		}else{
			return new FakeResultWrapper2(array());
		}
	}
}

class FakeResultWrapper2 extends FakeResultWrapper {
	function numRows() {
		$ret=parent::numRows();
//		print_stack_trace();
#		echo "FakeResultWrapper2::numRows($ret)\n";
		return $ret;
	}
	function fetchRow() {
#		echo "FakeResultWrapper2::fetchRow()\n";
#		print_stack_trace();
		return parent::fetchRow();
	}
	function fetchObject() {
#		echo "FakeResultWrapper2::fetchObject()\n";
//		print_stack_trace();
		return parent::fetchObject();
	}
}



$ourParser = new Parser();

function replaceImagesWithBase64Data($matches){
				//we embed the images
				if(file_exists($matches[1])){
					$file=fopen($matches[1], "r");
					$imgbinary = fread($file, filesize($matches[1]));
					$base64 = base64_encode($imgbinary);
					unset($imgbinary);
					fclose($file);
					return 'src="data:image/'.substr($matches[1],-3).';base64,'.$base64.'"';
					
				}else{
					return $matches[0];
				}
			}

function generateHtml($text,$title){
	global $ourParser;
	$ret = $ourParser->parse($text,Title::newFromText( $title ),new ParserOptions());

//			"<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" " .
//                       "\"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n" .
//                        "<html xmlns=\"http://www.w3.org/1999/xhtml\">\n" .

		   $ret="<html>\n".
                        "<head>\n" .
                        "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />\n" .
                        "<title>" . htmlspecialchars( $title ) . "</title>\n" .
                	'<link rel="stylesheet" href="/-/all.css" type="text/css" media="screen" />'.
                        "</head>\n" .
                        "<body>\n" .
                        preg_replace_callback('/src="([^"]*)"/','replaceImagesWithBase64Data',$ret->getText()) .
                        "</body>\n" .
                        "</html>";
#	echo $ret;
	return $ret;


}
