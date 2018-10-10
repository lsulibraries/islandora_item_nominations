<?php

/**
 *
 */

/**
 * Implements hook_form().
 */
function islandora_item_nominates_front_form() {
  $form = array();
  $form['front'] = array(
    '#type' => 'fieldset',
    '#title' => t('Choose items for front-page pool'),
  );
  $form['table'] = array(
    '#type' => 'tableselect',
    '#title => t('Front-page Pool'),
    '#description' => t('Choose items from the table to make them candidates for display on the front page),
    '#theme' => 'table',
  );
  return $form
}
