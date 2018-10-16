<?php

/**
 * @file
 *   Utility functions, check db, write or remove to database, and submit handler.
 */

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
      db_insert('islandora_item_nominations')
      ->fields(array('id', 'pid', 'dsid'))
      ->values(
        array(
          'id' => NULL,
          'pid' => $pid,
          'dsid' => $dsid,
        ))
        ->execute();
    }
    drupal_theme_rebuild();
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
    drupal_theme_rebuild();

}

function jpg_over_tn($object) {
  if (is_object($object['JPG'])) {
    return 'JPG';
  }
  else {
    return 'TN';
  }
}

/**
 * Submit handler to be added islandora_object_properties_form.
 *
 * @return function $fn
 * Returns either the nominate or denominate function.
 */
function islandora_item_nominations_submit_handler(&$form, &$form_state) {
  $obj = $form_state['object'];
  $dsid = jpg_over_tn($obj);
  if ($form_state['input']['form_id'] == 'islandora_object_properties_form') {
    $fn = $form_state['input']['nominate'] ? 'nominate' : 'denominate';
    $return = $form_state['input']['nominate'] ? $fn($form_state['values']['pid'], $dsid): $fn($form_state['values']['pid']);
    return $return;
  }
}

function add_context($pid, $context_id, $context_type) {
  db_insert('islandora_item_nominations')
    ->fields(array('context_id','context_type'))
    ->values(array(
      'contexnt_id' => $context_id,
      'context_type' => $context_type,
    ))
    ->condition('pid',$pid)
    ->execute();
    dsm("Context added, $context_type set to $context_id for $pid");
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
