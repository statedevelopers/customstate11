<?php
/**
 * @file
 * Functions for manipulating data within various entities. 
 *
 */
 
 /** ***************************************************************************
 * Array gathering functions.
 *
 */
 
  /**
   * Loads Regions vocabulary as objects.
   *
   */
    function customstate11_loadregions() {
      // LOAD REGIONAL TERMS
      $regions = array();
      $result = db_query("SELECT tid FROM {taxonomy_term_data} WHERE vid = 18;");
      foreach ($result as $record) {
        $regions[] = taxonomy_term_load($record->tid);
      }
      
      return($regions);
    }
  
  /**
   * Loads users as objects.
   *
   */
    function customstate11_loadusers() {
      // LOAD USERS
      $users = array();
      $result = db_query("SELECT uid FROM {users};");
      foreach ($result as $record) {
        $users[] = user_load($record->uid);
      }
      
      return($users);
    }
  
  /**
   * Loads Countries vocabulary as strings, keyed by tid.
   *
   */
    function customstate11_loadcountries() {
      // LOAD COUNTRY TERMS
      $countries = array();
      $result = db_query("SELECT tid, name FROM {taxonomy_term_data} WHERE vid = 16;");
      foreach ($result as $record) {
        $countries[$record->tid] = $record->name;
      }
      
      return($countries);
    }

  /**
   * Loads Countries vocabulary into array keyed by tid, with values of Region by tid.
   *
   */
    function customstate11_loadcountryregions() {
      // LOAD COUNTRY Parents
      $countryregions = array();
        $query  = "SELECT h.tid as tid, t.name as country, r.name as region, r.tid as rtid ";
        $query .= "FROM d7_gallery.taxonomy_term_hierarchy h ";
        $query .= "INNER JOIN d7_gallery.taxonomy_term_data t ";
        $query .= "  ON t.tid = h.tid ";
        $query .= "INNER JOIN d7_gallery.taxonomy_term_data p "; 
        $query .= "  ON p.tid = h.parent ";
        $query .= "INNER JOIN d7_gallery.taxonomy_term_data r "; 
        $query .= "  ON r.name = p.name ";
        $query .= "WHERE t.vid = 16 ";
        $query .= "  AND r.vid = 18 ";
        $query .= "ORDER BY r.name, t.name; ";

      $result = db_query($query);
      foreach($result as $record) {
        $countryregions[$record->tid] = $record->rtid;
      }
      
      return($countryregions);
    }
  
  
 /** ***************************************************************************
 * Node editing functions.
 *
 */
 
  /**
   * SET Country from Title and Description
   *
   */
    function customstate11_setnode_countryfromcontent($node, $countries, $changecount = 0) {
      if($node->field_country['und'][0]['tid']!= 1566) { // Set to default 'U.S.'
        $country = taxonomy_term_load($node->field_country['und'][0]['tid']);
print "Node <a href=\"/node/$node->nid\">$node->nid</a> has country '$country->name'.<br />\n";
        $title = $node->title;
        $desc = $node->field_description['und'][0]['value'];
        
        $title = str_ireplace('Myanmar', 'Burma', $title);
        $desc = str_ireplace('Myanmar', 'Burma', $desc);
        
        // Step through countries to find a match
        foreach($countries as $tid=>$country) {
          if($country=='United States') continue;
          if($country=='Africa (sub Sahara)') continue;
          if($country=='Domestic') continue;
          if($country=='International') continue;
          
          if(customstate11_strpos($title,$country,1)!==FALSE || customstate11_strpos($desc,$country,1)!==FALSE) {            
            // False Positives
            if($country=='Spain') {
              if(
                customstate11_strpos($title,"Port of Spain",1)!==FALSE 
                || customstate11_strpos($desc,"Port of Spain",1)!==FALSE
                || customstate11_strpos($title,"Port-of-Spain",1)!==FALSE 
                || customstate11_strpos($desc,"Port-of-Spain",1)!==FALSE
              ) {
                continue;
              }            
            }
          
            if($node->field_country['und'][0]['tid'] != $tid) {
print "-->Node <a href=\"/node/$node->nid\">$node->nid</a>: Key $key: Changing country to: '$country' (tid:$tid).<br />\n";
              $node->field_country['und'][0]['tid'] = $tid;
              node_save($node); $changecount++; // Don't use node_submit, which changes Author.
              break;
            }
          }         
        }
      }
    }
  
  /**
   * SET AUTHOR from field_auxauthor
   *
   */
    function customstate11_setnode_authorfromauxauthor($node) {
      // Must have auxiliary author field value && must not be assigned to any user other than Admin
      if(!empty($node->field_auxauthor['und'][0]['value']) && $node->uid == 1) {
        // If not admin, then verify the username. 
        $result = db_query("SELECT u.name FROM users u WHERE u.name = " . $node->);
        
        $auxauthor = $node->field_auxauthor['und'][0]['value'];
        // Exceptions for founding editors
        if($auxauthor == 'admin') $auxauthor = 'JonesEM'; // Eric Jones was the previous admin.
        if($auxauthor == 'lforgerson') $auxauthor = 'ForgersonLA'; 
        if($auxauthor == 'cjjackson') $auxauthor = 'JacksonJ2'; 
        if($auxauthor == 'Ruth Bennett') $auxauthor = 'BennettRE'; 
        if($auxauthor == 'Aaron Bruce') $auxauthor = 'abruce'; 
        if($auxauthor == 'PerryMD') $auxauthor = 'PerryMD2'; 
        if($auxauthor == 'adamss') $auxauthor = 'adamssc'; 
        if($auxauthor == 'johnsonha') $auxauthor = 'johnsonhm';
        
        // Verify a real user
        if($user = db_query('SELECT uid FROM {user} WHERE name = :name', array(':name' => $auxauthor))->fetchObject()) {
print "NODE: <a href=\"/node/$node->nid/edit\">$node->nid</a>: Changing author to: " . $user->uid . " -> '$user->name'.<br />\n";
          $node->uid = $user->uid;
          //node_save($node); return TRUE; // Don't use node_submit, which changes Author. 
        } else {
print "NODE: <a href=\"/node/$node->nid/edit\">$node->nid</a>: No user by the name of '$auxauthor'.<br />\n";
        }
      } else {
        if(empty($node->field_auxauthor['und'][0]['value'])) {
print "NODE: <a href=\"/node/$node->nid/edit\">$node->nid</a>: No auxiliary author data.<br />\n";
        } elseif ($node->uid != 1) {
print "NODE: <a href=\"/node/$node->nid/edit\">$node->nid</a>: Node is already assigned to user: $node->uid.<br />\n";
        }
      }
      
      return FALSE;
    }
  
  /**
   * SET REGION, based on keywords
   *
   */
    function customstate11_setnode_regionfromkeywords($node, $regions, $changecount = 0) {
      // Look for empty region, but with keywords
      if(empty($node->field_region['und'][0]['tid']) && !empty($node->field_keywords)) {
        $keywords = $node->field_keywords['und'][0]['value'];

        // SET REGION, based on keywords ---------------------------------------
        // Look for empty region, but with keywords
        if(empty($node->field_region['und'][0]['tid'])) { // && !empty($node->field_keywords)) {
          $country = taxonomy_term_load($node->field_country['und'][0]['tid']);
          $keywords = $node->field_keywords['und'][0]['value'];
          
          foreach($regions as $region) {
            $set_region = FALSE;
  //print "-->$region->tid: Compare: " . $keywords . " | " . $region->name . "<br />\n";
            if(stripos($keywords,$region->name)!==FALSE) {
              $set_region = TRUE;
  //print "-->MATCH: $region->tid: Compare: " . $keywords . " | " . $region->name . "<br />\n";
            } elseif(!empty($region->synonyms_synonym['und'])) {
              foreach($region->synonyms_synonym['und'] as $synonym) {
  //print "-->$region->tid: Compare synonym: " . $keywords . " | " . $synonym['value'] . "<br />\n";
                if (stripos($keywords,$synonym['value'])!==FALSE) {
                  $set_region = TRUE;
  //print "-->MATCH: $region->tid: Compare synonym: " . $keywords . " | " . $synonym['value'] . "<br />\n";              
                  break;
                }
              }
            }
            if($set_region) {
              if(!is_array($node->field_region['und']))$node->field_region['und'] = array();
             //if(!is_array($node->field_region['und'][0]))$node->field_region['und'][0] = array();
              $node->field_region['und'][0]['tid'] = $region->tid;
  print "$key: Setting region to: " . $node->field_region['und'][0]['tid'] . " -> '$region->name' on node '$node->nid'.<br />\n";
              node_save($node); $changecount++;
              break;          
            }
          }
        }
      }
    }

  /**
   * SET REGION, based on Country field
   *
   */
    function customstate11_setnode_regionfromcountry($node, $countryregions, $changecount = 0) {
      $regionfield = &$node->field_region['und'][0]['tid'];
      $countryfield = &$node->field_country['und'][0]['tid'];

      $country = taxonomy_term_load($countryfield);
      $region0 = taxonomy_term_load($regionfield);

      // Look for empty region, but with country that is NOT U.S., nor Africa (region), nor the Moon
      if(empty($regionfield)) {        
        // Detect conflicts
        $confirmed = FALSE;
        foreach($node->field_region['und'] as $regionarry) {
          if($regionarray['tid'] == $countryregions[$countryfield]) $confirmed = TRUE;
        }
        if($regionfield != $countryregions[$countryfield]) {
          $region1 = taxonomy_term_load($countryregions[$countryfield]);
print "NODE: <a href=\"/node/$node->nid/edit\">$node->nid</a>; CONFLICT $country->name -> <strong>$region0->name | $region1->name</strong><br />";
        }
        
        $regionfield = $countryregions[$countryfield];
        if(!empty($regionfield)) {
print "NODE: <a href=\"/node/$node->nid/edit\">$node->nid</a>; Setting $countryfield -> Region <strong>$regionfield</strong><br />";
          //$node->field_region['und'][0]['tid'] = $region->tid;
          node_save($node); $changecount++;
        }
      } else {
print "NODE: <a href=\"/node/$node->nid/edit\">$node->nid</a>; EMPTY $country->name -> <strong>No Region</strong><br />";
      }
    }

  /**
   * SET Creation, based on Date field
   *
   */
    function customstate11_setnode_creationfromdate($node, $changecount = 0) {
      $datefield = &$node->field_imagedate['und'][0]['value'];
      $date = strtotime($datefield);
      if($node->created > (strtotime('3/20/2012')-86400)) {
print "NODE: <a href=\"/node/$node->nid/edit\">$node->nid</a>: Created " . date('c',$node->created);
        $node->created = $date;
        node_save($node); $changecount++;
print ", changed to <strong>" . date('c',$node->created) . "</strong><br />\n";
      } else {
//print "NODE: <a href=\"/node/$node->nid/edit\">$node->nid</a>: Created " . date('c',$node->created) . "<br />\n";
      }
    }

  /**
   * SET Date, based on notes in description.
   *
   */
    function customstate11_setnode_datefromdesc($node, $changecount = 0) {
      $datefield = &$node->field_imagedate['und'][0]['value'];
      $desc = $node->field_description['und'][0]['value'];
//print "Desc = '$desc'<br />\n";
      $monthArray = array('January','February','March','April','May','June','July','August','September','October','November','December');
      $moArray = array('Jan.','Feb.','Mar.','Apr.','May','Jun.','Jul.','Aug.','Sep.','Oct.','Nov.','Dec.');
      
      // Scan for month name to get date.
      foreach($monthArray as $monkey=>$month) {
//print "Month '$month' and '" . $moArray[$monkey] . "'<br />\n";
        $monthpos = customstate11_strpos($desc, $month, 1);
        $mopos = customstate11_strpos($desc, $moArray[$monkey], 1);
        if($monthpos!==FALSE) {
          $pos = $monthpos;
        } elseif($mopos!==FALSE) {
          $pos = $mopos;
        } else {
          continue;
        }

        // Process
        $data = substr($desc, $pos);
        $data = str_replace('.','. ',$data);
        $data = str_replace(',','',$data);
        $data = str_replace('  ',' ',$data);
        $data = str_replace('  ',' ',$data);
        $data = str_replace('  ',' ',$data);
//print "Data: '$data' <br />\n";
        $dataArray = explode(' ',$data);
        $dateArray = array();
        for($i=0; $i<3; $i++) $dateArray[] = array_shift($dataArray);
//print "DateArray: '" . implode(' ',$dateArray) . "' <br />\n";
        
        // Qualify
        if(count($dateArray) != 3) continue;
//print "Flag a. Checking day '" . $dateArray[1] . "'<br />\n";
        if(!ctype_digit($dateArray[1])) continue;
//print "Flag b.<br />\n";
        if($dateArray[1] < 1 || $dateArray[1] > 31) continue;
        $dateArray[2] = str_replace('.','',$dateArray[2]);
//print "Flag c. Checking year '" . $dateArray[2] . "'<br />\n";       
        if(!ctype_digit($dateArray[2])) continue;
//print "Flag d.<br />\n";
        if($dateArray[2] < 1775 || $dateArray[2] > intval(date('Y'))) continue;
//print "Flag e: " . $dateArray[0] . " " . $dateArray[1] . ", " . $dateArray[2] . ".<br />\n";
        
        // Convert
        $date = strtotime($dateArray[0] . " " . $dateArray[1] . ", " . $dateArray[2]);
        
        // Verify conversion
        if($date < strtotime('1/1/1970') || $date > time()) continue;
        
        // Measure significance of delta
          // First convert field to integer, if not already. Date field cannot be saved in other formats without special functions. 
          $test = strtotime($datefield);
          if(!empty($test)) $datefield = $test;
        $diff = $datefield - $date;
        if($diff >= 86400) {
print "NODE: <a href=\"/node/$node->nid/edit\">$node->nid</a>: RESETTING DATE to '" . substr(date('c',$date),0,-6) . ", " . intval($diff/86400) . " days earlier than '" . substr(date('c',$datefield),0,-6) . "'.<br />\n";            
          $datefield = substr(date('c',$date),0,-6); 
          node_save($node); $changecount++;
          break;
//        } else {
//print "NODE: <a href=\"/node/$node->nid/edit\">$node->nid</a>: '" . date('c',$datefield) . "', DIFF: " . number_format($diff/3600,2) . " hours.<br />\n";            
        }
      }
    }

  /**
   * SET Date, based on exif metadata in the photo files.
   *
   */
    function customstate11_setnode_datefromexif($node, $changecount = 0) {
      $filename = $node->field_image['und'][0]['filename'];
      $path = '/var/www/html/sites/default/files/contributed_images';
      $filepath = $path . '/' . $filename;
      if(!file_exists($filepath)) {
        print "ERROR: File '$filename' cannot be found at '$filepath'.<br />\n";
        exit;
      }
      $exif = exif_read_data($filepath);
      if($exif !== FALSE) {
        $exifdate = $exif['FileDateTime'];
        $diff = $node->created - $exifdate; 
print "NODE: <a href=\"/node/$node->nid/edit\">$node->nid</a>: Exif date '" . date('c',$exifdate) . ", DIFF: '$diff'.<br />\n";
        
        // Take the earlier date. 
        if($diff >= 86400) {
print "NODE: <a href=\"/node/$node->nid/edit\">$node->nid</a>: RESETTING TO Exif date '" . date('c',$exifdate) . ", from '" . date('c',$node->created) . "'.<br />\n";
          $node->created = $exifdate; 
//          node_save($node); $changecount++;
        }
      } else {
print "NODE: <a href=\"/node/$node->nid/edit\">$node->nid</a>: No exif metadata available.<br />\n";
      }
    }

  /**
   * SET persons, based title, description.
   *
   */
    function customstate11_setnode_personfromcontent($node, $changecount = 0) {
      //$synonymArray = array('Hillary Rodham Clinton','Hillary Clinton','Secretary Clinton','Secretary of State Clinton');
      //$prime_tid = 1309;
      $synonymArray = array('Barack Obama','President Obama','Mr. Obama');
      $prime_tid = 1831;
      
      if(!is_array($node->field_persons)) $node->field_persons = array();
      if(count($node->field_persons)<1) $node->field_persons['und'] = array();
      
      $personfield = &$node->field_persons['und'];
      $content = $node->title . $node->field_description['und'][0]['value']; // Concat to save time.
      
      
      // Stop if term is already assigned.
      foreach($personfield as $term) {
        if($term['tid'] == $prime_tid) return;
      }
      
      
      /*
      // Kill spare terms.
      $found = FALSE;
      foreach($personfield as $pkey=>$term) {
        if($term['tid'] == $prime_tid) {
          if($found) {
            unset($personfield[$pkey]);
            $killed = TRUE;
print "NODE: <a href=\"/node/$node->nid/edit\">$node->nid</a>: Killing.<br />\n";
          } else {
            $found = TRUE;
          }
        }
      }
      if($killed) {
        node_save($node); $changecount++;
      }

      return;
      */
      

      // Scan for occurences of any synonym.
      foreach($synonymArray as $key=>$name) {
        if(customstate11_strpos($content, $name, 0)!==FALSE) {
          
print "NODE: <a href=\"/node/$node->nid/edit\">$node->nid</a>: Add term to '" . substr($content,customstate11_strpos($content, $name, 0), 40) . "...<br />\n";
          $personfield[] = array('tid'=>$prime_tid);
          node_save($node); $changecount++;
          break;
        }
      }
    }


  /**
   * SET persons, based title, description.
   *
   */
    function customstate11_setnode_dateformat($node, $changecount = 0) {
      $olddate = $node->field_imagedate['und'][0]['value'];
      if(ctype_digit($olddate)) {
        $node->field_imagedate['und'][0]['value'] = substr(date('c',$olddate),0,-6);
        $imagedatefield = $node->field_imagedate['und'][0]['value'];
print "NODE: <a href=\"/node/$node->nid/edit\">$node->nid</a>: ImageDate: '$olddate', change to '$imagedatefield'.<br>\n";
        node_save($node); $changecount++;
//      } else {
//print "NODE: <a href=\"/node/$node->nid/edit\">$node->nid</a>: Old date '$olddate' is ok.<br>\n";
      }
    }

  /**
   * .
   *
   */
    function customstate11_shownode($nid0, $nid1 = 0) {
        $nodes = array();
        $nodes[] = node_load($nid0); 
        if($nid1) {
          $nodes[] = node_load($nid1); 
        }
        
        foreach($nodes as $node) {
          print "NODE: <a href=\"/node/$node->nid/edit\">$node->nid</a>: ";
          print "ImageDate: '" . $node->field_imagedate['und'][0]['value'] . "'";
          print "<br>\n";
        }
        
        return(TRUE);
    }

  /**
   * Parse name into fullname (3 parts).
   *
   */
    function customstate11_setuser_fullname($user, $changecount = 0) {
      if(!empty($user->field_auxname['und'])) 
        $auxname = $user->field_auxname['und'][0]['value'];

      if(!empty($auxname)) {
        if(empty($user->field_fullname['und'][0]['family'])) {
          // Truncate multiple names separated by commas
          $generational = "";
          if (stripos($auxname, ',')) {
            // Exception for Jr.'s
            if (stripos($auxname, ', jr')) {
              $generational = "Jr.";
            }
            $auxname = substr($auxname,0,stripos($auxname, ','));
          }
          
          // Truncate multiple names separated by 'and'
          if (stripos($auxname, ' and ')) {
            $auxname = substr($auxname,0,stripos($auxname, ' and '));
          }
          
          $namearray = explode(' ',$auxname);
          
          // Recombine Family name prefixes, like 'von'
          while($part = current($namearray)) {
            if(strcasecmp($part, 'von') == 0) {
              $part .= " " . next($namearray);
            }
            $newarray[] = $part;
            next($namearray);
          }
          $namearray = $newarray;
          
          // Load name parts
          $family = array_pop($namearray);
          $given = array_shift($namearray);
          $middle = array_shift($namearray);
          $user->field_fullname['und'][0]['given'] = $given;
          $user->field_fullname['und'][0]['safe']['given'] = $given;
          $user->field_fullname['und'][0]['middle'] = $middle;
          $user->field_fullname['und'][0]['safe']['middle'] = $middle;
          $user->field_fullname['und'][0]['family'] = $family;
          $user->field_fullname['und'][0]['safe']['family'] = $family;        
          $user->field_fullname['und'][0]['generational'] = $generational;
          $user->field_fullname['und'][0]['safe']['generational'] = $generational;        
  
          $user->field_display_name['und'][0]['value'] = $user->field_auxname['und'][0]['value'];        
          
print "user: <a href=\"/user/$user->uid/edit\">$user->uid</a>: Fullname: '" . implode('|',$user->field_fullname['und'][0]['safe']) . "' from '" . $user->field_display_name['und'][0]['value'] . "'.<br>\n";
        } 
        if (empty($user->field_display_name['und'][0]['value'])) {
          $user->field_display_name['und'][0]['value'] = $user->field_auxname['und'][0]['value'];        
print "user: <a href=\"/user/$user->uid/edit\">$user->uid</a>: SET Display name: '" . $user->field_display_name['und'][0]['value'] . "'.<br>\n";
        } else {
print "user: <a href=\"/user/$user->uid/edit\">$user->uid</a>: Already SET Display name: '" . $user->field_display_name['und'][0]['value'] . "'.<br>\n";
        } 
        //user_save($user); $changecount++;
      }
    }

