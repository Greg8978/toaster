# Init for spencer project

#pdg set-up

# No spark (branch genom /mardi_dev2)
#rosservice call /pdg/manage_stream "{morseHuman: true, niutHuman: false, groupHuman: false, mocapHuman: false, pr2Robot: true,
#  spencerRobot: false, vimanObject: true, sparkObject: false, sparkFact: false}"

# spark (branch genom / mardi_dev2)
rosservice call /pdg/manage_stream "{morseHuman: false, niutHuman: false, groupHuman: false, mocapHuman: true, pr2Robot: true,
  spencerRobot: false, vimanObject: true, sparkObject: true, sparkFact: true}"

# no genom (branch master)
#rosservice call /pdg/manage_stream "{morseHuman: false, niutHuman: false, groupHuman: false, mocapHuman: true, pr2Robot: true,
#  spencerRobot: false}" 

#area manager setup
rosservice call /area_manager/add_area "myArea:
  id: 0
  name: 'Livingroom'
  myOwner: ''
  areaType: 'room'
  factType: ''
  entityType: 'entities'
  isCircle: false
  center: {x: 0.0, y: 0.0, z: 0.0}
  ray: 0.0
  poly:
    points:
    - {x: 2.3, y: 9.0, z: 0.0}
    - {x: 9.4, y: 9.0, z: 0.0}
    - {x: 9.4, y: 5.0, z: 0.0}
    - {x: 2.3, y: 5.0, z: 0.0}
    - {x: 2.3, y: 9.0, z: 0.0}
  insideEntities: [0]" 



rosservice call /area_manager/add_area "myArea:
  id: 0
  name: 'Kitchen'
  myOwner: ''
  areaType: 'room'
  factType: ''
  entityType: 'entities'
  isCircle: false
  center: {x: 0.0, y: 0.0, z: 0.0}
  ray: 0.0
  poly:
    points:
    - {x: 6.4, y: 13.1, z: 0.0}
    - {x: 9.4, y: 13.1, z: 0.0}
    - {x: 9.4, y: 9.0, z: 0.0}
    - {x: 6.4, y: 9.0, z: 0.0}
    - {x: 6.4, y: 13.1, z: 0.0}
  insideEntities: [0]"


rosservice call /area_manager/add_area "myArea:
  id: 0
  name: 'Bedroom'
  myOwner: ''
  areaType: 'room'
  factType: ''
  entityType: 'entities'
  isCircle: false
  center: {x: 0.0, y: 0.0, z: 0.0}
  ray: 0.0
  poly:
    points:
    - {x: 6.4, y: 9.0, z: 0.0}
    - {x: 6.4, y: 13.1, z: 0.0}
    - {x: 2.4, y: 13.1, z: 0.0}
    - {x: 2.4, y: 9.0, z: 0.0}
    - {x: 6.4, y: 9.0, z: 0.0}
  insideEntities: [0]"


rosservice call /area_manager/add_area "myArea:
  id: 0
  name: 'Livingroom_coffeetable'
  myOwner: ''
  areaType: 'support'
  factType: ''
  entityType: 'entities'
  isCircle: true
  center: {x: 4.9, y: 7.3, z: 0.0}
  ray: 2.0
  poly:
    points:
    - {x: 0.0, y: 0.0, z: 0.0}
  insideEntities: [0]"

rosservice call /area_manager/add_area "myArea:
  id: 0
  name: 'Livingroom_table'
  myOwner: ''
  areaType: 'support'
  factType: ''
  entityType: 'entities'
  isCircle: true
  center: {x: 7.7, y: 6.8, z: 0.0}
  ray: 2.0
  poly:
    points:
    - {x: 0.0, y: 0.0, z: 0.0}
  insideEntities: [0]"

rosservice call /area_manager/add_area "myArea:
  id: 0
  name: 'Kitchen_table'
  myOwner: ''
  areaType: 'support'
  factType: ''
  entityType: 'entities'
  isCircle: true
  center: {x: 8.1, y: 9.8, z: 0.0}
  ray: 2.0
  poly:
    points:
    - {x: 0.0, y: 0.0, z: 0.0}
  insideEntities: [0]"

rosservice call /area_manager/add_area "myArea:
  id: 0
  name: 'interaction'
  myOwner: 'pr2'
  areaType: ''
  factType: 'pr2_interaction'
  entityType: 'agents'
  isCircle: false
  center: {x: 0.0, y: 0.0, z: 0.0}
  ray: 2.0
  poly:
    points:
    - {x: 0, y: -1, z: 0}
    - {x: 2, y: -2, z: 0}
    - {x: 2, y: 2, z: 0}
    - {x: 0, y: 1, z: 0}
    - {x: 0, y: -1, z: 0}
  insideEntities: [0]" 

rosservice call /area_manager/publish_all_areas

