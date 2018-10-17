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
 * Checks db for entry of a given pid.
 *
 * @param string $pid
 * Object pid
 */
function is_nominated($pid) {
  $records = db_select('islandora_item_nominations', 'n')
    ->fields('n', array('pid'))
    ->condition('pid', $pid)
    ->execute();
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
    return; //already nominated
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
  db_delete('islandora_item_nominations')
    ->condition('pid', $pid)
    ->execute();
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



function context_update($pid, $fields, $id_condition = FALSE) {
  $query = db_update('islandora_item_nominations');
  if($id_condition != FALSE) {
    $query->condition('id' ,$id_condition['id']);
  }
  $query->fields($fields)
    ->condition('pid', $pid)
    ->execute();
  if (isset($fields['context_type'])) {
      $context_type = $fields['context_type'];
      $context_id = $fields['context_id'];
      dsm("Context added, $context_type set to $context_id for $pid");
  }
  else {
    $bool = $fields['front'] ? 'true' : 'false';
    dsm("Context added for $pid, front page pool candidacy set to $bool");
  }
}

function get_nominated($namespace = FALSE) {
  $query = db_select('islandora_item_nominations', 'n');
  if ($namespace) {
    $query->condition('pid', $namespace . '%', 'LIKE');
  }
    $query->fields('n');
    $results = $query->execute();
  $options = array();
  foreach ($results as $result) {
    $options[$result->id] = (array) $result;
    $options[$result->id]['pid-link'] = l($result->pid, 'islandora/object/' . $result->pid);
  }
  return $options;
}

function update_collection_ds($coll_pid) {
  $coll_ns = explode(':', $coll_pid);
  $coll_ns = $coll_ns[0];
  $results = db_select('islandora_item_nominations', 'n')
    ->fields('n')
    ->condition('pid', $coll_ns.'%', 'LIKE')
    ->condition('context_id', $coll_pid)
    ->execute()
    ->fetchAll();
    foreach($results as $result) {
      $object = islandora_object_load($result->pid);
      $ds = $object[$result->dsid];
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
  module_load_include('inc','islandora','includes/utilities');
    if($model == 'children of compound object types'){return $model;}
    module_load_include('inc', 'islandora', 'includes/utilities');
    $not_cmobj = $model != 'fedora-system:ContentModel-3.0';
    $not_fobj = $model != 'fedora-system:FedoraObject-3.0';
    if (($not_fobj) && ($not_cmobj)) {
        $cmodel_map = islandora_content_stats_islandora_get_content_models();
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
