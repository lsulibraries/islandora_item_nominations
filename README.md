# Item Nominations

This module provides an apparatus for _nominating_ items so that their JPG/TN is available to various contexts.

## Description

### Terms

* **context**: some logical space in which nominated images will be used (collection page, front page)
* **pool**: pool of images available for rotation on any given context
* **key image**: image chosen from the pool for a context (randomized from actively nominated items)
* **namespace prefix***: when the `islandora_namespace_homepage` module is enabled, namespace prefix is a context additional to _front_ or _collection_

### Routes

1. The following paths are significant:
  1. `/islandora/object/%pid%/manage/properties`
    * alter the properties form to include a checkbox.
  2. `/islandora/object/%collection-pid%/manage/collection/nominations`
    * provides a select select list that allows a collection admin to choose a key image for the collection
  3. `/%ns-prefix%/manage`*
    * when the module `islandora_namespace_homepage` is installed, this route provides a select list that allows an institutional admin to choose a key image to represent the institution
  4. `/admin/islandora/tools/nominations`
    * provides a [tableselect](https://api.drupal.org/api/drupal/includes%21form.inc/function/theme_tableselect/7.x) containing all images nominated with JPGs for pool of potential key images for front page context.

### Schema

3 databases. 

islandora_item_nominations_context_front
islandora_item_nominations_context_namespace
islandora_item_nominations_context_collection

* **pid**: _string_ islandora/fedora pid
* **dsid**: _string_ datastream ID (JPEG, TN)
* **context_id**: _string_ unique id for the context, like  a pid (ie _my-stuff:collection_) (Not required for front context.)
* **context_type** _string_ collection, cpd, namespace
* **indicator**: _bool_ whether or not this item is actively nominated for it's context

### Constraints / Requirements / Rules

1. prefer JPG datastream, use TN as a fallback
2. metadata from MODS will be title, description, creator, date, (cmodel is not part of metadata cmodel but pulled from the RI)
3. `is_eligible()` determines item eligibility; nominate control will only be rendered for items that
  1. are not embargoed
  2. are published (toggle_unpublish module_exists)
  3. do not have generic image/tn (audio, that folder icon)
4. front-page pool will only include pids with high-quality images (no TN)
5. a default will be used as key image on first installation and in cases where no suitable item exists (audio collections) (!!! Unimplemented...)
6. front-page pool membership can be vetoed by privileged users (super admins) (!!!! check this...)
7. Items nominated, will be available at the parent collection pool, the namespace pool, and if a JPG exists: the front page pool.
8. Nominated items, must be activated at any given context.

### API
The following functions and return values will be needed:

* `includes/utilities.inc get_nominated_for_context($context_type, $context_id = FALSE, $load = FALSE)`
  * `$context_type` _string_ which context, corresponds to db table.
  * `$context_id` _string_ which pid or namespace the current page is in.
  * `$load` _string_ if string: only return actively nomitated items for with idicator of 1 to be displayed.
  * `return $options`
  * `$options` _array_ set of db records `WHERE context_id = current_context`
* `includes/utilities.inc nominate($pid, $context)`
  * should be called when a new item is chosen as a potential nominee for any context.
  * `$pid` _string_ dora/fedora PID
  * db_insert into all 3 tables* 
  * `includes/utilities.inc denominate($pid, $context)`
  * should be called when a new item is romoved as a potential nominee for any context.
  * `$pid` _string_ dora/fedora PID
  * db_delete into all 3 tables* 
* `include/utilities.inc context_update_context_id($pid, $fields)`
  * update the db record for a pid, setting a value for a context
  * `$pid` _string_ dora/fedora PID
  * `$fields` _array_ Database fields for context_type table.
  * db_update indicator to the opposite of it's current setting. (Activate a candidate item)
* `get_image_metadata($pid)`
  * return [$title, $description, $creator, $date_created, $cmodel]

### Forms
* item-level nominate checkbox (hook_form_islandora_object_properties_form_alter)
* context-level select list for setting key item (hook other forms: namespace admin_form_alter,  hook_islandora_basic_collection_build_manage_object with a new form) 
* front-page pool management form (hook_menu)

## * for consideration (at a later date...)
* this module is potentially valuable outside of LSU, decouple this from our namespace-prefix machinery
  * instead, let ns-homepage use this module

## * Update to planning.
* To make things simpler and more versatile, we proposed multiple db tables per context. We use 3 db tables, one per context. 
* steps to go through
  * item is chosen for nomination at object/manage/properties
  * item is now a potential candidate for collection, namespace and homepage contexts. ~~and any other context.~~
  * visit a context admin page, ie admin_form, collection/manage, or islandora/tools/front-page-pools
  * choose from potential candidates which will be used and update the respective context's database.
  * context displays both potential candidates and nominated items.
  * removing an object's nomination from object/manage/properties removes the obj from all 3 databases.
