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
