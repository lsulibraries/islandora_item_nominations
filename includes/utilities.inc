<?php

/**
 * @file
 *   Utility functions, check db, write or remove to database, and submit handler.
 */

function is_eligible($pid) {
  //check ip embargo
  $ograbme = db_query("SELECT * from islandora_ip_embargo_embargoes where pid = :pid", array(':pid' => $pid));
  $ip_embargoed = $ograbme->rowCount() > 0;
  $notlisted = db_query("SELECT * FROM islandora_collection_toggle_publish_collections where pid = :pid", array(':pid' => $pid));
  $published = $notlisted->rowCount() == 0;
  if ($ip_embargoed) {
    return FALSE;
  }
  elseif (!$published) {
    //check that pid is not listed in islandora_collection_toggle_publish_collections
    return FALSE;
  }
  else {
    return TRUE;
  }
}

/**
 *
 * Checks db for entry of a given pid, optional arguments for context id and type
 *
 * @param string $pid
 * Object pid
 */
function is_nominated($pid, $context_id = FALSE, $context_type = FALSE) {
  $fields = $context_id ? array('pid', 'context_id', 'context_type') : array('pid');
  $table = 'islandora_item_nominations';
  if ($context_type) {
    $table .= "_context_$context_type";
  }
  $query = db_select($table, 'n')
    ->fields('n', $fields)
    ->condition('pid', $pid);
  if($context_id) {
    $query->condition('context_id', $context_id);
  }
    $records = $query->execute();
    $nominated = $records->rowCount();
    return $nominated >= 1;
}

/**
 *
 * Adds given pid with dsid to db.
 *
 * @param string $pid
 *   Object pid.
 * @param string $dsid
 *   Prefered datastream id (JPG or TN if no JPG)
 */
function nominate($pid, $dsid) {
  $check = is_nominated($pid);
  if ($check) {
    //already nominated
    return;
  }
  else {
    $values = array(
      'id' => NULL,
      'pid' => $pid,
      'dsid' => $dsid,
    );
    if ($dsid == 'TN') {
      $values['front'] = '0';
    }
    db_insert('islandora_item_nominations')
    ->fields(array('id', 'pid', 'dsid'))
    ->values($values)
    ->execute();
  }
}

/**
 *
 * Removes given pid from db.
 *
 * @param string $pid
 * Object pid
 */
function denominate($pid) {
  //adapt to remove from multiple tables
  db_delete('islandora_item_nominations')
    ->condition('pid', $pid)
    ->execute();
}

//new for multi table
function context_update_context_id($pid, $fields) {
  $c_type = $fields['context_type'];
  $c_id = isset($fields['context_id']) ? $fields['context_id'] : FALSE;
  //trace the condition back...
  $condition = is_nominated($pid, $c_id, $c_type);
  $db_call = $condition == FALSE ? 'db_insert' : 'db_delete';
  $state = $condition == FALSE ? 'added' : 'deleted';
  $fields = $c_id ? array('pid' => $pid, 'context_id' => $c_id): array('pid' => $pid);
  $update = $db_call("islandora_item_nominations_context_$c_type");
  if ($db_call == 'db_delete') {
    $update->condition('pid', $pid);
    if($c_id){
      $update->condition('context_id', $c_id);
    }
  }
  else {
    $update->fields($fields);
  }
  $update->execute();
  dsm("Context '$c_type' $state for $pid.");
}

//old way
//works great for namespace homepage.
function get_nominated($namespace = FALSE, $front = FALSE) {
  $query = db_select('islandora_item_nominations', 'n');
  if ($namespace) {
    $query->condition('pid', $namespace . '%', 'LIKE');
  }
    $query->fields('n');
    $results = $query->execute();
  $options = array();
  foreach ($results as $result) {
    $options[$result->id] = (array) $result;
    $pid = $result->pid;
    $id = $result->id;
    $options[$id]['pid-link'] = l($pid, "islandora/object/$pid");
  }
  return $options;
}

//context can be a collection, ns_homepage, or frontpage, or any user defined context
//not used at the namespace homepage page, bc the old function works so well.
function get_nominated_for_context($context_type, $context_id = FALSE) {
  //this is more exact and can include 'front' as a context ID or any other user defined context
  $query = db_select("islandora_item_nominations_context_$context_type", 'c');
  if ($context_id) {
    $query->condition('context_id', $context_id);
  }
  //join this table with the datastream of the nominated item.
  $query->innerjoin('islandora_item_nominations', 'n', 'c.pid = n.pid');
  $query->fields('n', array('pid', 'dsid'));
  $query->fields('c');
  $results = $query->execute();
  $options = array();
  foreach ($results as $result) {
    $options[$result->pid] = (array) $result;
    $pid = $result->pid;
    //$id = $result->id;
    $options[$pid]['pid-link'] = l($pid, "islandora/object/$pid");
  }
  return $options;
}

//not really a great fn, we don't want to update the TN every time we visit root, or ns homepage
function update_collection_ds($coll_pid) {
  $coll_ns = explode(':', $coll_pid);
  $coll_ns = $coll_ns[0];
  $results = db_select('islandora_item_nominations_context_collection', 'c');
  $results->fields('c', array('pid', 'context_type','context_id'));
  $results->innerjoin('islandora_item_nominations', 'n', 'c.pid = n.pid');
  $results->fields('n', array('pid', 'dsid'));
  $results->condition('context_id', $coll_pid);
  $result = $results->execute();
  $candidates = $result->fetchAll();
  foreach($candidates as $nom) {
    $object = islandora_object_load($nom->pid);
    $ds = $object[$nom->dsid];
    if (isset($ds)) {
      $coll_obj = islandora_object_load($coll_pid);
      $TN = $coll_obj['TN'];
      $TN->setContentFromString($ds->content);
    }
    else {
      drupal_set_message('Datastream not valid', 'error');
    }
  }
}

/**
 * Loads image metadata.
 *
 *@param abstract $object
 *  Preloaded object
 */
function get_image_metadata($object) {
  $mods = simplexml_load_string($object['MODS']->content);
  return array(
    'title' => property_exists($mods, 'titleInfo') ? (string) $mods->titleInfo->title : t('Title Info Title is missing'),
    'description' => property_exists($mods, 'abstract') ? (string) $mods->abstract : t('Description missing'),
    'creator' => property_exists($mods, 'creator') ? $mods->creator : t('Creator missing'),
    'date_created' => property_exists($mods, 'date_created') ? $mods : t('Date created missing'),
    'cmodel' => nominations_map_cmodels($object->models[0])
  );
}

/**
 * Maps a machine name to a human readable $label
 * @param string $model
 *  String of model's machine name
 * @return string
 *  String of model's label stripped of repetitous words
 *
 *  Borrowed from islandora_content_stats
 */
function nominations_map_cmodels($model) {
    if($model == 'children of compound object types'){return $model;}
    module_load_include('inc', 'islandora', 'includes/utilities');
    $not_cmobj = $model != 'fedora-system:ContentModel-3.0';
    $not_fobj = $model != 'fedora-system:FedoraObject-3.0';
    if (($not_fobj) && ($not_cmobj)) {
        $cmodel_map = islandora_get_content_models();
        if ($model != '' && array_key_exists($model, $cmodel_map)) {
            $label = $cmodel_map[$model];
            $replace = $label['label'];
            $l_replace = str_replace('Large ', '', $replace);
            $i_replace = str_replace('Islandora ', '', $l_replace);
            $short_label = str_replace(' Content Model', '', $i_replace);
            return $short_label;
        }
        return $model;
    }
    return $model;
}

function jpg_over_tn($object) {
  if (is_object($object['JPG'])) {
    return 'JPG';
  }
  elseif (is_object($object['TN'])) {
    return 'TN';
  }
  else {
    drupal_set_message('Object is lacking a valid datastream, try a child object instead or upload a thumbnail or a jpeg', 'error');
  }
}
