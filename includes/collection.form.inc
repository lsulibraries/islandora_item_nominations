<?php

/**
 * @file Forms for collection level nominated items.
 */
function islandora_item_nominations_collection_form($form, &$form_state) {
$form = array();
$coll_ns = explode(':', $form_state['build_info']['args']['0']->id);
$coll_ns = $coll_ns[0];
module_load_include('inc', 'islandora_item_nominations', 'includes/utilities')
$options = get_nominated($coll_ns);

$form['label'] = array(
  '#type' => 'fieldset',
  '#title' => t('Choose items, to give collection specific context'),
);
$headers = array(
  'pid-link' => array('data' => t('Item pid'), 'field' => 'n.pid'),
  'dsid' => array('data' => t('Datastream'), 'field' => 'n.dsid'),
  'context_id' => array('data' => t('Context Id (pid or namespace)'), 'field' => 'n.context_id'),
  'context_type' => array('data' => t('Type of Context'), 'field' => 'n.context_type'),
  'front' => array('data' => t('Show on frontpage'), 'field' => 'n.front'),
);
$form['table'] = array(
  '#type' => 'tableselect',
  '#title' => t('Nominate collection items'),
  '#description' => t('Choose items to be featured on the collection background/thumbnail image.'),
  '#header' => $headers,
  '#options' => $options,
  '#empty' => t('No items added yet, to nominate visit islandora/object/%object:pid%/manage/properties'),
);
$form['submit'] = array(
  '#type' => 'submit',
  '#value' => t('Nominate for collection'),
);
return $form;
}

/**
 *  Sets context and context_id, overwrites existing.
 */
function islandora_item_nominations_collection_form_submit(&$form, &$form_state) {
  module_load_include('inc', 'islandora_item_nominations', 'includes/utilities');
  foreach ($form_state['input']['table'] as $id) {
    $pid = $form_state['complete form']['table']['#options'][$id]['pid'];
    $fields = array(
      'context_type' => 'collection',
      'context_id' => $form_state['build_info']['args']['0']->id,
    );
    $id_condition = array('id' => $id);
    context_update($pid, $fields, $id_condition);
    update_collection_ds($fields['context_id']);
  }
}
