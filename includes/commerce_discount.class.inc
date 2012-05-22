<?php

/**
 * Entity class representing the commerce_discount entity type.
 */
class CommerceDiscount extends Entity {

  /**
   * The machine name of the discount offer.
   *
   * @var string
   */
  public $name;

  /**
   * The label of the discount.
   *
   * @var string
   */
  public $label;

  /**
   * Set the default discount type (bundle).
   *
   * @var string
   */
  public $type = 'order_discount';

  /**
   * Whether the discount is enabled.
   *
   * @var bool
   */
  public $enabled = TRUE;

  public function __construct($values = array()) {
    parent::__construct($values, 'commerce_discount');
  }
}
