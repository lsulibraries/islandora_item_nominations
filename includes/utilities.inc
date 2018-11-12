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
function is_nominated($pid, $context_id = FALSE) {
  $tables = array('front', 'namespace', 'collection');
  $fields = array('pid');
  $result = array();
  foreach ($tables as $table) {
    $db_table = "islandora_item_nominations_context_$table";
    $query = db_select($db_table, 'n')
      ->fields('n', $fields)
      ->condition('pid', $pid);
    $records = $query->execute();
    $result[] = $records->rowCount();
  }
    return !in_array('0', $result);
}

function is_candidate($pid, $context_id, $context_type) {
  $fields = array('pid', 'context_id', 'indicator') ;
  $table = "islandora_item_nominations_context_$context_type";
  $query = db_select($table, 'n')
    ->fields('n', $fields)
    ->condition('pid', $pid)
    ->condition('context_id', $context_id)
    ->execute();
  $results = $query->fetchAll();
  return $results[0]->indicator;
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
  $check = is_nominated($pid, 'collection');
  if ($check) {
    //already nominated
    return;
  }
  else {
    $values = array(
      'pid' => $pid,
      'dsid' => $dsid,
      'indicator' => 0,
    );
    if ($dsid != 'TN') {
    db_insert('islandora_item_nominations_context_front')
      ->fields(array('pid', 'dsid', 'indicator'))
      ->values(array($values))
      ->execute();
    }
    $ns_str = explode('-', $pid);
    array_pop($ns_str);
    $ns = implode($ns_str);
    $values['context_id'] = $ns.'-';
    db_insert('islandora_item_nominations_context_namespace')
      ->fields(array('pid', 'dsid', 'indicator', 'context_id'))
      ->values(array($values))
      ->execute();
    $coll = explode(':', $pid);
    $coll = $coll[0]. ':collection';
    $values['context_id'] = $coll;

    db_insert('islandora_item_nominations_context_collection')
      ->fields(array('pid', 'dsid', 'indicator', 'context_id'))
      ->values(array($values))
      ->execute();
    dsm("Item $pid was nominated.");
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
  db_delete('islandora_item_nominations_context_front')
    ->condition('pid', $pid)
    ->execute();
  db_delete('islandora_item_nominations_context_namespace')
    ->condition('pid', $pid)
    ->execute();
  db_delete('islandora_item_nominations_context_collection')
    ->condition('pid', $pid)
    ->execute();
        dsm("Item $pid is no longer nominated.");

}

/**
 *  @param string $pid
 *
 *  @param array $fields
 *    database fields for context.
 */
function context_update_context_id($pid, $fields) {
  $c_type = $fields['context_type'];
  $c_id = isset($fields['context_id']) ? $fields['context_id'] : FALSE;
  $indicator = is_candidate($pid, $c_id, $c_type);
  $indicator = $indicator == '0' ? 1 : 0;

  $state = $indicator == TRUE ? 'added' : 'deleted';
  $fields = $c_id ? array('pid' => $pid, 'context_id' => $c_id) : array('pid' => $pid);
  $update = db_update("islandora_item_nominations_context_$c_type");
  $update->fields(array('indicator' => $indicator));
  $update->execute();
  dsm("Context '$c_type' $state for $pid.");
}

//context can be a collection, ns_homepage, or frontpage, or any user defined context
function get_nominated_for_context($context_type, $context_id = FALSE, $load = FALSE) {
  //this is more exact and can include 'front' as a context ID or any other user defined context
  $query = db_select("islandora_item_nominations_context_$context_type",'c')
    ->fields('c');
  if ($load) {
    $query->condition('indicator', 1);
  }
  if ($context_type == 'namespace') {
    $query->condition('pid', $context_id . '%', 'LIKE');

  }
  else {
    $query->condition('context_id', $context_id);
  }
  $results = $query->execute();
  $options = array();
  foreach ($results as $result) {
    $pid = $result->pid;
    $options[$pid] = (array) $result;
    $options[$pid]['pid-link'] = l($pid, "islandora/object/$pid");
  }
  return $options;
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
    'title' => property_exists($mods, 'titleInfo') ? (string) $mods->titleInfo->title : '',
    'description' => property_exists($mods, 'abstract') ? (string) $mods->abstract : '',
    'creator' => property_exists($mods, 'creator') ? $mods->creator : '',
    'date_created' => property_exists($mods, 'date_created') ? $mods : '',
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
